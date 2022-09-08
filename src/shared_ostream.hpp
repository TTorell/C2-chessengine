#ifndef __SHARED_OSTREAM
#define __SHARED_OSTREAM

#include <thread>
#include <mutex>
#include <iostream>
#include "chessfuncs.hpp"

//// Declaration of the concept "arithmetic", which
//// is satisfied by any type 'T' such that for values 'a'
//// of type 'T', are integral or floating point numbers.
//#include <concepts>
//#include <type_traits>
//#include <cstddef>
//template<typename T>
//concept arithmetic = requires(T a)
//{
//  requires std::is_arithmetic_v<T>;
//};
namespace
{

// The chessboard prints some utf-8 cgaracters.
// So, I figured it would be good if the whole logfile
// has the same character format.
std::string iso_8859_1_to_utf8(const std::string& str)
{
  std::string str_out;
  for (const char ch : str)
  {
    uint8_t byte = static_cast<uint8_t>(ch);
    if (byte < 0x80)
    {
      str_out.push_back(static_cast<char>(byte));
    }
    else
    {
      str_out.push_back(static_cast<char>(0xc0 | (byte >> 6)));
      str_out.push_back(static_cast<char>(0x80 | (byte & 0x3f)));
    }
  }
  return str_out;
}

} // End of fileprivate namespace

namespace C2_chess
{
// I got tired of sending in the log-file as an
// argument to every function which needed it.
// and I didn't want to make it a global variable.
// So I thought, why not make it a singleton.
// So:
//
// A Class with probably thread-safe methods with
// which we can write strings, which will be converted
// to UTF-8 encoded text-strings, to an output-stream
// from different threads. (There are some UTF-8
// chess-figures I use and it's better if the whole
// file is in UTF-8 than to mix character encodings.)
//
// It's written as a "multiple singleton"-pattern.
// You can receive one instance for writing to an
// ostream of your choice and one instance designated
// for writing on cout (stdout).
//
// In the first case, to specify the ostream and get
// a pointer to an instance of this class, you must
// first call:
// get_instance(ostream& os, bool is_open);, but in
// consecutive calls you can use get_instance(); without
// arguments.
//
// For cout you simply call get_cout_instance();
//
// If _is_open is false we will never try to write anything
// to the ostream. So we don't have to remove the writings
// from the user-code to stop the output.
// We can just call the close()-method of this class.
// And you can send in the ofstream.is_open() in the
// first get_instance-call when you try to open an output
// file, e.g.:
// ofstream ofs("command_log.txt");
// Shared_ostream& logfile = *(Shared_ostream::get_instance(ofs, ofs.is_open()));
// And we'll never write anything if the ofs failed to open.
//
// When I come to a point in the main-function where I know
// The program will be used as a chess-engine (when cout is
// designated to sending commands to the chess-gui with) and
// not as a commandline-tool I can do:
// Shared_ostream* cmd_line = Shared_ostream::get_cout_instance();
// cmd_line->close();
// And all write-attempts to cout via my singleton instance
// *cmd_line will just be "swallowed". The chess engine output-
// thread will use cout directly.
//
// The instance of the embedded ostream must naturally live
// as long as any code is using the Shared_ostrem instance.
// or we must use the close() method when the ostream dies.
// TODO: Maybe something to check dynamically in the class.
// instead of checking _is_open. Didn,t think of that,
// but all ostreams doesn't have an is_open() method.

class Shared_ostream
{
  private:
    static Shared_ostream* p_instance_;
    static Shared_ostream* p_cout_instance_;
    std::ostream& _os;
    bool _is_open;

    Shared_ostream();

    Shared_ostream(std::ostream& os, bool is_open) :
        _os(os),
        _is_open(is_open)
    {
    }

    ~Shared_ostream()
    {
    }

  public:
    static std::mutex static_mutex;

    // Singletons should not be cloneable or assignable
    void operator=(const Shared_ostream&) = delete;
    Shared_ostream(Shared_ostream& other) = delete;

    // Create and/or return a pointer to the singleton-instance of the Shared_ostream.
    static Shared_ostream* get_instance(std::ostream& os, bool is_open)
    {
      // lock the execution of this method, so no other thread can call this method
      // simultaneously, then make sure again that the variable is null and then
      // call the constructor.
      std::lock_guard<std::mutex> lock(static_mutex);
      if (p_instance_ == nullptr)
      {
        p_instance_ = new Shared_ostream(os, is_open);
      }
      return p_instance_;
    }

    // This method may not be called until get_instance(ostream& os, bool is_open)
    // has been called at least once to create the singleton *p_instance.
    static Shared_ostream* get_instance()
    {
      std::lock_guard<std::mutex> lock(static_mutex);
      return p_instance_;
    }

    // Create and/or return a pointer to the singleton-instance of the cout-Shared_ostream.
    static Shared_ostream* get_cout_instance()
    {
      // lock the execution of this method, so no other thread can call this method
      // simultaneously, then make sure again that the variable is null and then
      // call the constructor.
      std::lock_guard<std::mutex> lock(static_mutex);
      if (p_cout_instance_ == nullptr)
      {
        p_cout_instance_ = new Shared_ostream(std::cout, true);
      }
      return p_cout_instance_;
    }

    void close()
    {
      _is_open = false;
    }

    void open()
    {
      _is_open = true;
    }

    template<typename T>
    Shared_ostream& operator<<(const T& var)
    {
      if (_is_open)
      {
        std::stringstream ss;
        ss << var;
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
      }
      return *this;
    }

    // Method for a string which is already UTF-8 coded.
    void write_UTF8_string(std::string s)
    {
      if (_is_open)
      {
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << s << std::flush;
      }
    }

//    // Template only for primitive arithmetic types (float, double, char, short, int etc).
//    // It works, but isn't necessary. The first template, above, covers primitives too.
//    // And having both templates defined clashes with:
//    // error: ambiguous overload for ‘operator<<’ (operand types are ‘C2_chess::Shared_ostream’ and ‘double’).
//    template <typename T>
//    requires arithmetic<T>
//    Shared_ostream& operator<<(T value)
//    {
//      // std::cerr << std::is_arithmetic_v<T> << std::endl;
//      if (_is_open)
//      {
//        std::stringstream ss;
//        ss << value;
//        std::lock_guard<std::mutex> locker(static_mutex);
//        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
//      }
//      return *this;
//    }

//    Tried to make it work with the std::endl-function which returns an ostream&
//    It works when calling endl with an argument
//    ( e.g."Shared_ostream instance" << std::endl(cout); ), but that's not good enough.
//    The problem, I think, is that Shared_ostream "isn't" an ostream, it just "has"
//    an ostream.
//    Shared_ostream& operator<<(ostream& os)
//    {
//      std::lock_guard < std::mutex > locker(static_mutex);
//      if (_is_open)
//        _os << std::endl << std::flush;
//      return *this;
//    }

//    // Template for writing the contents of a vector with each element on the same line.
//    // But that can be done before sending the vector to the Shared_ostream, e.g. via a
//    // stringstream.
//    template<typename T>
//    Shared_ostream& operator<<(const std::vector<T>& v)
//    {
//      if (_is_open)
//      {
//        std::stringstream ss;
//        write_vector(v, ss, true);
//        std::lock_guard<std::mutex> locker(static_mutex);
//        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
//      }
//      return *this;
//    }
};

} // namespace C2_chess


#endif

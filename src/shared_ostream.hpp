#ifndef __SHARED_OSTREAM
#define __SHARED_OSTREAM

#include <thread>
#include <mutex>
#include <iostream>
#include <concepts>
#include <type_traits>
#include <cstddef>
#include "chessfuncs.hpp"
#include "movelog.hpp"
#include "bitboard.hpp"
#include "pgn_info.hpp"

// Declaration of the concept "arithmetic", which
// is satisfied by any type 'T' such that for values 'a'
// of type 'T', are integral or floating point numbers.
template<typename T>
concept arithmetic = requires(T a)
{
  requires std::is_arithmetic_v<T>;
};

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
class Bitmove;
class Config_parms;
std::ostream& operator<<(std::ostream& os, const Bitmove& m);
std::ostream& operator<<(std::ostream& os, const Config_params& params);

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

    Shared_ostream& operator<<(const std::string& s)
    {
      if (_is_open)
      {
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << iso_8859_1_to_utf8(s) << std::flush;
      }
      return *this;
    }

//    Shared_ostream& operator<<(int i)
//    {
//      std::lock_guard<std::mutex> locker(static_mutex);
//      if (_is_open)
//        _os << iso_8859_1_to_utf8(std::to_string(i)) << std::flush;
//      return *this;
//    }
//
//    Shared_ostream& operator<<(float flt)
//    {
//      std::lock_guard<std::mutex> locker(static_mutex);
//      if (_is_open)
//        _os << iso_8859_1_to_utf8(std::to_string(flt)) << std::flush;
//      return *this;
//    }
//
//    Shared_ostream& operator<<(double dbl)
//    {
//      std::lock_guard<std::mutex> locker(static_mutex);
//      if (_is_open)
//        _os << iso_8859_1_to_utf8(std::to_string(dbl)) << std::flush;
//      return *this;
//    }
//
//    Shared_ostream& operator<<(long l)
//    {
//      std::lock_guard<std::mutex> locker(static_mutex);
//      if (_is_open)
//        _os << iso_8859_1_to_utf8(std::to_string(l)) << std::flush;
//      return *this;
//    }
//
//    Shared_ostream& operator<<(unsigned long ul)
//    {
//      std::lock_guard<std::mutex> locker(static_mutex);
//      if (_is_open)
//        _os << iso_8859_1_to_utf8(std::to_string(ul)) << std::flush;
//      return *this;
//    }

    template <typename T>
    requires arithmetic<T>
    Shared_ostream& operator<<(T value)
    {
      std::cerr << std::is_arithmetic_v<T> << std::endl;
      if (_is_open)
      {
        std::stringstream ss;
        ss << value;
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
      }
      return *this;
    }

//    Tried to make it work with the endl-function which returns an ostream&
//    It works when calling endl with an argument
//    ( e.g."Shared_ostream instance" << endl(cout); ), but that's not good enough.
//    Shared_ostream& operator<<(ostream& os)
//    {
//      os << "\n";
//      std::lock_guard < std::mutex > locker(static_mutex);
//      if (_is_open)
//        _os << std::endl << std::flush;
//      return *this;
//    }

    Shared_ostream& operator<<(const Movelog& ml)
    {
      std::lock_guard<std::mutex> locker(static_mutex);
      if (_is_open)
      {
        std::stringstream ss;
        ml.write(ss);
        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
      }
      return *this;
    }

    template<typename T>
    Shared_ostream& operator<<(const std::vector<T>& v)
    {
      if (_is_open)
      {
        std::stringstream ss;
        write_vector(v, ss, true);
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
      }
      return *this;
    }

    Shared_ostream& operator<<(const Bitmove& m)
    {
      if (_is_open)
      {
        std::stringstream ss;
        ss << m;
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
      }
      return *this;
    }

    Shared_ostream& operator<<(const Config_params& params)
    {
      if (_is_open)
      {
        std::stringstream ss;
        ss << params;
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

    void write_search_info(const Search_info& si, const std::string& pv_line)
    {
      if (_is_open)
      {
        std::stringstream ss;
        ss << "\nEvaluated on depth:" << static_cast<int>(si.max_search_depth) << " " << static_cast<int>(si.leaf_node_counter) << " nodes in " << si.time_taken << " milliseconds." << std::endl;
        ss << "PV_line: " << pv_line << "score: " << si.score << std::endl;
        ss << "beta_cutoffs: " << static_cast<int>(si.beta_cutoffs) << " first_beta_cutoffs: " << static_cast<float>(si.first_beta_cutoffs) / static_cast<float>(si.beta_cutoffs)
            * 100.0F
           << "%"
           << std::endl;
        ss << "hash_hits:" << static_cast<int>(si.hash_hits) << std::endl;
        ss << "highest search_ply:" << static_cast<int>(si.highest_search_ply) << std::endl;
        std::lock_guard<std::mutex> locker(static_mutex);
        _os << iso_8859_1_to_utf8(ss.str()) << std::flush;
      }
    }
};

} // namespace C2_chess
#endif

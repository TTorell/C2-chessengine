#ifndef __SHARED_OSTREAM
#define __SHARED_OSTREAM

#include <thread>
#include <mutex>
#include <iostream>
#include "move.hpp"
#include "movelist.hpp"
#include "chessfuncs.hpp"
#include "config_param.hpp"

using namespace std;

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
// to writing on cout (stdout).
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
// instead of checking _is_open. Didn,t think of that.

class Shared_ostream {
  private:
    static Shared_ostream* p_instance_;
    static Shared_ostream* p_cout_instance_;
    // Mutexes for static functions
    static mutex mutex_;
    static mutex cout_mutex_;
    // Mutex for non-static member functions
    mutex _mu;
    ostream& _os;
    bool _is_open;

    Shared_ostream();

    Shared_ostream(ostream& ost, bool is_open) :
        _os(ost),
        _is_open(is_open)
    {
    }

    ~Shared_ostream()
    {
    }

  public:
    // Singletons should not be cloneable or assignable
    void operator=(const Shared_ostream&) = delete;
    Shared_ostream(Shared_ostream& other) = delete;

    static Shared_ostream* get_instance(ostream& os, bool is_open);

    static Shared_ostream* get_instance();

    static Shared_ostream* get_cout_instance();

    void close()
    {
      _is_open = false;
    }

    void open()
    {
      _is_open = true;
    }

    Shared_ostream& operator<<(const string& s)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(s) << flush;
      return *this;
    }

    Shared_ostream& operator<<(int i)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(i)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(float f)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(f)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(double d)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(d)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(long l)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(l)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(unsigned long ul)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(ul)) << flush;
      return *this;
    }


//    Tried to make it work with the endl-function which returns an ostream&
//    It works when calling endl with an argument
//    ( e.g."Shared_ostream instance" << endl(cout); ), but that's not good enough.
//    Shared_ostream& operator<<(ostream& os)
//    {
//      os << "\n";
//      lock_guard < mutex > locker(_mu);
//      if (_is_open)
//        _os << endl << flush;
//      return *this;
//    }

    Shared_ostream& operator<<(const Movelog& ml)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
      {
        stringstream ss;
        ss << ml << endl;
        _os << iso_8859_1_to_utf8(ss.str()) << flush;
      }
      return *this;
    }

    void log_time_diff(uint64_t nsec_stop,
                       uint64_t nsec_start,
                       int search_level,
                       const Move& best_move,
                       float score);

    // Method for a string which is already UTF-8 coded.
    void write_UTF8_string(string s)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
        _os << s << flush;
    }

    void write_config_params(const Config_params& params)
    {
      lock_guard < mutex > locker(_mu);
      if (_is_open)
      {
        _os << endl << iso_8859_1_to_utf8("Configuration paramaers:") << endl;
        _os << iso_8859_1_to_utf8(params.get_params_string()) << endl;
      }
    }
};
} // namespace
#endif

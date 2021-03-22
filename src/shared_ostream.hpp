
#ifndef __SHARED_OSTREAM
#define __SHARED_OSTREAM

#include <thread>
#include <mutex>
#include <iostream>
#include "move.hpp"
#include "chessfuncs.hpp"
#include "config_param.hpp"

using namespace std;

namespace C2_chess
{

class Shared_ostream {
  private:
    static Shared_ostream* pinstance_;
    static std::mutex mutex_;
    mutex _mu;
    ostream& _os;
    bool _is_open;

    Shared_ostream();

    Shared_ostream(ostream& ost, bool is_open) :
        _os(ost), _is_open(is_open)
    {
    }

    ~Shared_ostream() {}

  public:
    // Singletons should not be cloneable or assignable
    void operator=(const Shared_ostream&) = delete;
    Shared_ostream(Shared_ostream &other) = delete;

    static Shared_ostream* get_instance(ostream& os, bool is_open);

    static Shared_ostream* get_instance();

    Shared_ostream& operator<<(const string& s)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(s) << flush;
      return *this;
    }

    Shared_ostream& operator<<(int i)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(i)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(float f)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(f)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(double d)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(d)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(long l)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(l)) << flush;
      return *this;
    }

    Shared_ostream& operator<<(unsigned long ul)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << iso_8859_1_to_utf8(to_string(ul)) << flush;
      return *this;
    }

    void log_time_diff(uint64_t nsec_stop,
                       uint64_t nsec_start,
                       int search_level,
                       const Move& best_move,
                       float score);

    void write_UTF8_string(string s)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
        _os << s << flush;
    }

    void write_config_params(const Config_params& params)
    {
      lock_guard<mutex> locker(_mu);
      if (_is_open)
      {
       _os << endl << iso_8859_1_to_utf8("Configuration paramaers:") << endl;
       _os << iso_8859_1_to_utf8(params.get_params_string()) << endl;
      }
    }
};
} // namespace
#endif

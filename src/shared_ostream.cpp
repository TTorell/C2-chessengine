#include "shared_ostream.hpp"


namespace C2_chess
{

Shared_ostream* Shared_ostream::p_instance_{nullptr};
Shared_ostream* Shared_ostream::p_cout_instance_{nullptr};

mutex Shared_ostream::mutex_;
mutex Shared_ostream::cout_mutex_;

// Create and/or return a pointer to the singleton-instance of the Shared_ostream.
Shared_ostream* Shared_ostream::get_instance(ostream& os, bool is_open)
{
  // lock the execution of this method, so no other thread can call this method
  // simultaneously, then make sure again that the variable is null and then
  // call the constructor.
  lock_guard<mutex> lock(mutex_);
  if (p_instance_ == nullptr)
  {
    p_instance_ = new Shared_ostream(os, is_open);
  }
  return p_instance_;
}

// This method may not be called until get_instance(ostream& os, bool is_open)
// has been called at least once to create the singleton *p_instance.
Shared_ostream* Shared_ostream::get_instance()
{
  lock_guard<mutex> lock(mutex_);
  return p_instance_;
}

// Create and/or return a pointer to the singleton-instance of the cout-Shared_ostream.
Shared_ostream* Shared_ostream::get_cout_instance()
{
  // lock the execution of this method, so no other thread can call this method
  // simultaneously, then make sure again that the variable is null and then
  // call the constructor.
  lock_guard<mutex> lock(cout_mutex_);
  if (p_cout_instance_ == nullptr)
  {
    p_cout_instance_ = new Shared_ostream(cout, true);
  }
  return p_cout_instance_;
}

void Shared_ostream::log_time_diff(uint64_t nsec_stop,
                                   uint64_t nsec_start,
                                   int search_level,
                                   const Move& best_move,
                                   float score)
{
  uint64_t timediff = (nsec_stop - nsec_start);
  // Log the time it took;
  string s = "time spent by C2 on search level " + to_string(search_level) + " " + to_string(timediff/1.0e6) +
      " " + best_move.bestmove_engine_style() + " score = " + to_string(score);
  _os << iso_8859_1_to_utf8(s) << "\n";
}

} // namespace C2_chess

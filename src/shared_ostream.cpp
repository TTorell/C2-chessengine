#include "shared_ostream.hpp"


namespace C2_chess
{

Shared_ostream* Shared_ostream::pinstance_{nullptr};

std::mutex Shared_ostream::mutex_;

// Create and/or return the singleton-instance of the Shared_ostream.
Shared_ostream* Shared_ostream::get_instance(ostream& os, bool is_open)
{
  // lock the execution of this method, so no other thread can call this method
  // simultaneously, then make sure again that the variable is null and then
  // call the constructor.
  lock_guard<std::mutex> lock(mutex_);
  if (pinstance_ == nullptr)
  {
    pinstance_ = new Shared_ostream(os, is_open);
  }
  return pinstance_;
}

// This method may not be called until get_instance(ostream& os, bool is_open)
// has been called at least once.
Shared_ostream* Shared_ostream::get_instance()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return pinstance_;
}

void Shared_ostream::log_time_diff(uint64_t nsec_stop, uint64_t nsec_start, int search_level, const Move& best_move, float score)
{
  uint64_t timediff = (nsec_stop - nsec_start);
  // Log the time it took;
  string s = "time spent by C2 on search level " + to_string(search_level) + " " + to_string(timediff/1.0e6) +
      " " + best_move.bestmove_engine_style() + " score = " + to_string(score);
//  ostringstream ss;
//  ss << timediff / 1.0e6;
//  string value(ss.str());
//  ostringstream ss2;
//  ss2 << score;
//  string score_value(ss2.str());
  _os << iso_8859_1_to_utf8(s) << endl;
}

} // namespace C2_chess

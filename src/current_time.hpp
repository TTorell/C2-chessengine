/*
 * current_time.hpp
 *
 *  Created on: 20 nov. 2018
 *      Author: torsten
 */

#ifndef CURRENT_TIME_H
#define CURRENT_TIME_H

#include <chrono>
#include <cstdint>
namespace C2_chess
{

class Current_time
{
  private:
    std::chrono::steady_clock _clock;
    uint64_t _start_time_ns;
    uint64_t _stop_time_ns;
  public:
    inline uint64_t milliseconds()
    {
      return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(_clock.now().time_since_epoch()).count());
    }

    inline uint64_t microseconds()
    {
      return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(_clock.now().time_since_epoch()).count());
    }

    inline uint64_t nanoseconds()
    {
      return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(_clock.now().time_since_epoch()).count());
    }

    inline void tic()
    {
      _start_time_ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(_clock.now().time_since_epoch()).count());
    }

    inline uint64_t toc_ns()
    {
      _stop_time_ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(_clock.now().time_since_epoch()).count());
      return _stop_time_ns - _start_time_ns;
    }

    inline uint64_t toc_us()
    {
      _stop_time_ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(_clock.now().time_since_epoch()).count());
      return (_stop_time_ns - _start_time_ns)/1000;
    }

    inline uint64_t toc_ms()
    {
      _stop_time_ns = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(_clock.now().time_since_epoch()).count());
      return (_stop_time_ns - _start_time_ns)/1000000;
    }
};
}
#endif  /* CURRENT_TIME_H */


/*
 * Circularfifo.h
 *
 *  Created on: 22 jan. 2019
 *      Author: torsten
 */

#ifndef CIRCULAR_FIFO_HPP_
#define CIRCULAR_FIFO_HPP_

#include <mutex>
#include <iostream>
#include "shared_ostream.hpp"

static const int list_size = 256;

namespace C2_chess
{

class Circular_fifo
{
  protected:
    std::mutex _mu;
    std::string _list[list_size];
    unsigned long _put_index;
    unsigned long _get_index;

  public:
    Circular_fifo() :
        _mu(),
        _put_index(0),
        _get_index(0)
    {
    }

    virtual ~Circular_fifo()
    {
    }

    void put(const std::string &s)
    {
      std::lock_guard<std::mutex> locker(_mu); // unlocks when it goes out of scope
      if (_put_index - _get_index < list_size)
      {
        _list[_put_index % list_size] = s;
        _put_index++;
      }
    }

    std::string get()
    {
      std::string s;
      std::lock_guard<std::mutex> locker(_mu);
      if (_get_index < _put_index)
      {
        s = _list[_get_index % list_size];
        _get_index++;
        return s;
      }
      else
        return "";
    }
};
}

#endif /* CIRCULAR_FIFO_HPP_ */

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
using namespace std;

static const int size = 256;

namespace C2_chess
{

class Circular_fifo {
  protected:
    mutex _mu;
    string _list[size];
    unsigned long _put_index;
    unsigned long _get_index;

  public:
    Circular_fifo() :
        _put_index(0), _get_index(0)
    {
    }

    virtual ~Circular_fifo()
    {
    }

    void put(const string& s)
    {
      lock_guard<mutex> locker(_mu); // unlocks when it goes out of scope
      if (_put_index - _get_index < size)
      {
        _list[_put_index % size] = s;
        _put_index++;
      }
    }

    string get()
    {
      string s;
      lock_guard<mutex> locker(_mu);
      if (_get_index < _put_index)
      {
        s = _list[_get_index % size];
        _get_index++;
        return s;
      }
      else
        return "";
    }

};

class Shared_ostream {
  private:
    Shared_ostream();

  protected:
    mutex _mu;
    ostream& os;

  public:
    Shared_ostream(ostream& ost) :
        os(ost)
    {
    }

    Shared_ostream& operator<<(const string& s)
    {
      lock_guard<mutex> locker(_mu);
      os << s << flush;
      return *this;
    }

    Shared_ostream& operator<<(int i)
    {
      lock_guard<mutex> locker(_mu);
      os << i << flush;
      return *this;
    }

    Shared_ostream& operator<<(float f)
    {
      lock_guard<mutex> locker(_mu);
      os << f << flush;
      return *this;
    }

    Shared_ostream& operator<<(long l)
    {
      lock_guard<mutex> locker(_mu);
      os << l << flush;
      return *this;
    }

};
}

#endif /* CIRCULAR_FIFO_HPP_ */

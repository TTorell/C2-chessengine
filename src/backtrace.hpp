/*
 * backtrace.hpp
 *
 *  Created on: 8 dec. 2018
 *      Author: torsten
 */

#ifndef SRC_BACKTRACE_HPP_
#define SRC_BACKTRACE_HPP_
using namespace std;

#include <iostream>

class Backtrace
{
  public:
    Backtrace();
    ~Backtrace();
    friend ostream& operator<<(ostream& os, const Backtrace& bt);

    ostream& print(ostream& os) const;
    void print() const;
};

#endif /* SRC_BACKTRACE_HPP_ */

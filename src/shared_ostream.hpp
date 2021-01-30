
#ifndef __SHARED_OSTREAM
#define __SHARED_OSTREAM

#include <thread>
#include <mutex>
#include <iostream>

using namespace std;

namespace C2_chess
{

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

    Shared_ostream& operator<<(float fl)
    {
      lock_guard<mutex> locker(_mu);
      os << fl << flush;
      return *this;
    }

    Shared_ostream& operator<<(long l)
    {
      lock_guard<mutex> locker(_mu);
      os << l << flush;
      return *this;
    }
};
} // namespace
#endif

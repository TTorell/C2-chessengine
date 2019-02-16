#ifndef _RANK
#define _RANK

#include <iostream>
#include "chessfuncs.hpp" // only for require()

namespace C2_chess
{

class Square;

using std::ostream;

class Rank {
  protected:
    Square* _file[8];
    char _name;

  public:
    Rank() :
        _name('?')
    {
      for (int i = a; i <= h; i++)
      {
        _file[i] = 0;
      }
    }
    ~Rank()
    {
    }
//    char get_name()
//    {
//      return _name;
//    }
    void set_name(char name)
    {
      _name = name;
    }
    int get_rankindex()
    {
      return ((int) _name - 48);
    }
    Square*& operator[](int index) const
    {
      require(index >= a && index <= h, __FILE__, __FUNCTION__, __LINE__);
      return (Square*&) _file[index];
    }
};
}
#endif

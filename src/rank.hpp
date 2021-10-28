#ifndef _RANK
#define _RANK

#include "chessfuncs.hpp" // only for require()

namespace C2_chess
{

class Square;

class Rank
{
  protected:
    Square *_file[8];
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

    Rank(const Rank&) :
        _file{ nullptr },
        _name('?')
    {
    }

    ~Rank()
    {
    }
    //    char get_name()
//    {
//      return _name;
//    }

    Rank operator=(const Rank&) = delete;

    void set_name(char name)
    {
      _name = name;
    }
    int get_rankindex()
    {
      return ((int)_name - 48);
    }
    Square*& operator[](int index) const
    {
      require(index >= a && index <= h, __FILE__, __FUNCTION__, __LINE__);
      return const_cast<Square*&>(_file[index]);
    }
};
}
#endif

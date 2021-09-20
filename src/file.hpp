#ifndef _FIL
#define _FIL

#include "square.hpp"
#include "chessfuncs.hpp" //only for require()

namespace C2_chess
{

class File {
  protected:
    Square* _rank[9];
    char _name;

  public:
    File() :
        _name('?')
    {
      for (int i = 1; i <= 8; ++i)
      {
        _rank[i] = 0;
      }
    }
    ~File()
    {
      for (int i = 1; i <= 8; ++i)
      {
        if (_rank[i])
        {
          delete _rank[i];
          //_rank[i] = 0;
        }
      }
    }
    void name(char name)
    {
      _name = name;
    }
    int fileindex()
    {
      return ((int) _name - 97);
    }
    Square*& operator[](int index) const
    {
      require(index > 0 && index < 9, __FILE__, __FUNCTION__, __LINE__);
      return const_cast<Square*&>(_rank[index]);
    }
};
}
#endif

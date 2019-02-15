#include "file.hpp"
#include "square.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

namespace C2_chess
{

File::File() :
    _name('?')
{
  //cerr<<"default constructor File\n";
  for (int i = 1; i <= 8; ++i)
  {
    _rank[i] = 0;
  }
}

File::File(char name) :
    _name(name)
{
  //cerr<<"constructor File\n";
  for (int i = 1; i <= 8; ++i)
  {
    _rank[i] = 0;
  }
}

File::~File()
{
  for (int i = 1; i <= 8; ++i)
  {
    if (_rank[i])
    {
      delete _rank[i];
      _rank[i] = 0;
    }
  }
}

Square*& File::operator[](int index) const
{
  require(index > 0 && index < 9,
  __FILE__,
                       __FUNCTION__,
                       __LINE__);
  // Unfortunately we can't return a const Square*
  // So we cast away the const
  return const_cast<Square*&>(_rank[index]);
}

bool File::is_included(Square* compared_square) const
{
  for (int index = 1; index <= 8; index++)
    if (compared_square == _rank[index])
      return true;
  return false;
}

ostream& operator<<(ostream& os, const File& f)
{
  os << f._name;
  return os;
}
}

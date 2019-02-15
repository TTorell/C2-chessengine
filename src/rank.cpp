#include "rank.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

namespace C2_chess
{

Rank::Rank(char name):_name(name)
{
	for (int i=a; i<=h; i++)
	{
		_file[i]=0;
	}
}

Rank::Rank():_name('?')
{
	for (int i=a; i<=h; i++)
	{
		_file[i]=0;
	}
}

Rank::~Rank()
{
}

Square*& Rank::operator[](int index) const
{
    require(index >= a && index <= h, __FILE__, __FUNCTION__, __LINE__);
	return (Square*&)_file[index];
}

bool Rank::is_included(Square* compared_square) const
{
  for (int index=a;index<=h;index++)
	  if (compared_square==_file[index])
		  return true;
  return false;
}

ostream& operator<<(ostream& os, const Rank& r)
{
	os << r._name;
	return os;
}
}

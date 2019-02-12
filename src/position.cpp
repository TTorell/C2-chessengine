#include "chesstypes.hpp"
Position::Position() :
    _file(a), _rank(1)
{
}

Position::Position(int file, int rank) :
    _file(file), _rank(rank)
{
}

Position::Position(const Position& p) :
    Position(p._file, p._rank)
{
}

Position::~Position()
{
}

Position& Position::operator=(const Position& p)
{
  _rank = p._rank;
  _file = p._file;
  return *this;
}

bool Position::operator==(const Position& p) const
{
  return _file == p._file && _rank == p._rank;
}

ostream& operator<<(ostream& os, const Position& p)
{
  switch (p._file)
  {
    case 0:
      os << 'a';
      break;
    case 1:
      os << 'b';
      break;
    case 2:
      os << 'c';
      break;
    case 3:
      os << 'd';
      break;
    case 4:
      os << 'e';
      break;
    case 5:
      os << 'f';
      break;
    case 6:
      os << 'g';
      break;
    case 7:
      os << 'h';
      break;
    default:
      cerr << "Illegal filename in Position::operator<<" << endl;
      require(false, __FILE__, __func__, __LINE__);
  }
  os << p._rank;
  return os;
}

bool Position::same_diagonal(const Position& p) const
{
  return abs(_file - p._file) == abs(_rank - p._rank);
}

/*bool Position::same_file(const Square& s) const
 {
 return _file==s.get_position()._file;
 }


 bool Position::same_rank(const Square& s) const
 {
 return _file==s.get_position()._file;
 }


 bool Position::same_diagonal(const Square& s) const
 {
 return abs(_file - s.get_position()._file) == abs(_rank - s.get_position()._rank);
 }
 */

#include "position.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

namespace C2_chess
{

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

int Position::get_file() const
{
  return _file;
};

int Position::get_rank() const
{
  return _rank;
}

bool Position::set_file(char filechar)
{
  _file=filechar-'a'; return (filechar >='a' && filechar <='h');
}

bool Position::set_file(int fileindex)
{
  _file=fileindex;
  return (fileindex >=a && fileindex <=h);
}

bool Position::set_rank(char rankchar)
{
  _rank=rankchar + 1 - '1';
  return (rankchar >='1' && rankchar <='8');
}

bool Position::set_rank(int rankindex)
{
  _rank=rankindex; return (rankindex >=1 && rankindex <=8);
}

bool Position::same_file(const Position& p) const
{
  return _file==p._file;
}

bool Position::same_rank(const Position& p) const
{
  return _rank==p._rank;
}

std::ostream& operator<<(std::ostream& os, const Position& p)
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
      std::cerr << "Illegal filename in Position::operator<<" << std::endl;
      require(false, __FILE__, __func__, __LINE__);
  }
  os << p._rank;
  return os;
}

bool Position::same_diagonal(const Position& p) const
{
  return abs(_file - p._file) == abs(_rank - p._rank);
}
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

#include <iomanip>
#include <deque>
#include <iterator>
#include <sstream>
#include "movelist.hpp"
#include "move.hpp"
#include "chessfuncs.hpp"
#include "shared_ostream.hpp"
namespace C2_chess
{

  Movelist::Movelist() :
    _listindex(0)
{
}

Movelist::Movelist(const Movelist& ml) :
    _listindex(ml._listindex)
{
  for (Move* obj : ml._list)
  {
    _list.push_back(new Move(*obj));
  }
}

Movelist::~Movelist()
{
  for (Move* obj : _list)
    delete obj;
  _list.clear();
}

Movelist& Movelist::operator=(const Movelist& ml)
{
  for (Move* obj : _list)
    delete obj;
  _list.clear();
  for (Move* obj : ml._list)
  {
    _list.push_back(new Move(*obj));
  }
  _listindex = 0;
  return *this;
}

int Movelist::size() const
{
  return (int) _list.size();
}

Move* Movelist::first() const
{
  if (_list.empty())
    return 0;
  Movelist* tmp_this = const_cast<Movelist*>(this);
  tmp_this->_listindex = 1; //point to the second element
  return _list[0]; //return the first element
}

Move* Movelist::next() const
{
  Movelist* const tmp_this = const_cast<Movelist*>(this);
  if (_listindex < (int) _list.size())
  {
    Move* const tmp_move = _list[tmp_this->_listindex++];
    return tmp_move;
  }
  else
    return 0;
}

void Movelist::into(Move* const newmove)
{
  if (newmove->get_capture())
    _list.push_front(new Move(newmove));
  else
    _list.push_back(new Move(newmove));
}

void Movelist::into_as_last(Move* const newmove)
{
  _list.push_back(new Move(newmove));
}

void Movelist::into_as_first(Move* const newmove)
{
  _list.push_front(new Move(newmove));
}

void Movelist::out(Move* const rubbish)
{
  for (std::deque<Move*>::iterator it = _list.begin(); it != _list.end(); it++)
  {
    if (*it == rubbish)
    {
      _list.erase(it);
      break;
      // The _listindex variable will be set to point to the preceeding
      // element (thus decreased by 1) if the element to be removed (rubbish)
      // is placed before the object being pointed out by _listindex,
      // or if _listindex actually points out the rubbish-element.

      // Attention! The internal pointer to the rubbish-object
      // is lost.
      // So the memory which rubbish points at must be deleted
      // outside this method.

      // TODO: What about list_index
    }
  }
}

bool Movelist::empty() const
{
  return (_list.empty());
}

void Movelist::clear()
{
  for (Move* obj : _list)
    delete obj;
  _list.clear();
  _listindex = 0;
}

bool Movelist::in_list(const Move& m, int& index) const
{
  for (index = 0; index < (int)_list.size(); index++)
  {
    if (*_list[index] == m)
      return true;
  }
  index = -1;
  return false;
}

Move* Movelist::operator[](int i) const
{
  if ((i < (int) _list.size()) && (i >= 0))
    return _list[i];
  else
  {
    std::cout << "index " << i << " out of bounds error in Movelist[], size = " << _list.size() << std::endl;
#ifdef __linux__
    print_backtrace(std::cerr) << std::endl;
#endif // linux

    for (auto it : _list)
      std::cout << *(Move*) (it) << std::endl;
    exit(0);
  }
}

void Movelog::set_first_moveno(int moveno)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  logfile << "Setting first_moveno to " << moveno << "\n";
  _first_moveno = moveno;
}

std::ostream & Movelist::write(std::ostream & os, bool same_line) const
{
  for (auto it : _list)
  {
    os << *it;
    if (same_line)
      os << " ";
    else
      os << std::endl;
  }
  return os;
}

std::ostream & Movelog::write(std::ostream & os) const
{
  int moveno = _first_moveno;
  int increment = 0;
  for (int i = 0; i < (int) _list.size(); i++)
  {
    std::ostringstream move;
    move << *_list[i];
    if (i == 0 && _col_to_start == col::black)
    {
      os << moveno++ << "." << std::left << std::setw(9) << "  ...  " << move.str() << std::endl;
      increment = 1;
      continue;
    }

    if ((i + increment) % 2 == 0)
    {
      os << moveno++ << "." << std::left << std::setw(9) << move.str();
      if (i == (int) (_list.size() - 1))      // last move
        os << std::endl;
    }
    else
    {
      os << move.str() << std::endl;
    }
  }
  return os;
}
}

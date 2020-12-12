#include <iomanip>
#include <deque>
#include <iterator>
#include "movelist.hpp"
#include "move.hpp"
#include "backtrace.hpp"

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

int Movelist::cardinal() const
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
  if (newmove->get_take())
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
  for (deque<Move*>::iterator it = _list.begin(); it != _list.end(); it++)
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

bool Movelist::in_list(Move* m, int* index) const
{
  for (*index = 0; *index < (int) _list.size(); (*index)++)
  {
    if (*_list[*index] == m)
      return true;
  }
  *index = -1;
  return false;
}

Move* Movelist::operator[](int i) const
{
  if ((i < (int) _list.size()) && (i >= 0))
    return _list[i];
  else
  {
    cout << "index " << i << " out of bounds error in Movelist[], size = " << _list.size() << endl;
    Backtrace bt;
    bt.print();
    for (auto it : _list)
      cout << *(Move*) (it) << endl;
    exit(0);
  }
}

ostream & Movelist::write(ostream & os) const
{
  for (auto it : _list)
  {
    os << *it << endl;
  }
  return os;
}

ostream & Movelog::write(ostream & os) const
{
  int moveno = 1;
  int increment = 0;
  for (int i = 0; i < (int) _list.size(); i++)
  {
    ostringstream move;
    move << *_list[i];
    if (i == 0 && _col_to_start == black)
    {
      os << moveno++ << "." << left << setw(9) << "  ...  " << move.str() << endl;
      increment = 1;
      continue;
    }

    if ((i + increment) % 2 == 0)
    {
      os << moveno++ << "." << left << setw(9) << move.str();
      if (i == (int) (_list.size() - 1))      // last move
        os << endl;
    }
    else
    {
      os << move.str() << endl;
    }
  }
  return os;
}
}

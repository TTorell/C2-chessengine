#include "squarelist.hpp"
#include "chessfuncs.hpp"

namespace C2_chess
{

Squarelist::Squarelist() :
    _no_of_elements(0),
    _listindex(0)
{
}

Squarelist::~Squarelist()
{
}

Square* Squarelist::first() const //pseudo const, changes _listindex
{
  if (_no_of_elements > 0)
  {
    Squarelist* tmp_this = const_cast<Squarelist*>(this); //Casting away the const.
    tmp_this->_listindex = 1; //Point to the second element
    return _list[0]; // return the first element
  }
  return 0;
}

Square* Squarelist::next() const //also pseudo const
{
  if (_listindex < _no_of_elements)
  {
    Squarelist* tmp_this = const_cast<Squarelist*>(this);
    return _list[tmp_this->_listindex++];
  }
  return 0;
}

void Squarelist::into(Square* const newsquare)
{
  if (_no_of_elements < SQUARELISTMAX)
    _list[_no_of_elements++] = (Square*) newsquare;
  else
  {
    std::cerr << "squarelist full" << std::endl;
    std::cerr << _no_of_elements << " " << SQUARELISTMAX << std::endl;
//    for (int i = 0; i < _no_of_elements; i++)
//      _list[i]->write_name(cerr);
    require(false, __FILE__, __func__, __LINE__);
  }
}

void Squarelist::out(Square* const rubbish)
{
  bool found = false;
  for (int i = 0; i < _no_of_elements; i++)
  {
    if (_list[i] == rubbish)
    {
      found = true;
      _no_of_elements--;
      // The _listindex variable will be set to point to the preceding
      // element (decreased by 1) if the element to be removed (rubbish)
      // is placed before the object being pointed out by _listindex,
      // or if _listindex actually points to the rubbish-element
      if (i <= _listindex)
        _listindex--;
    }
    if (found) // move the following elements one step down in the list
      _list[i] = _list[i + 1]; // The internal pointer to the rubbish-object is lost
  }
}

int Squarelist::cardinal() const
{
  return _no_of_elements;
}

bool Squarelist::empty() const
{
  return (_no_of_elements == 0);
}

bool Squarelist::full() const
{
  return (_no_of_elements > SQUARELISTMAX);
}

bool Squarelist::in_list(Square* const s) const
{
  for (int i = 0; i < _no_of_elements; i++)
  {
    if (_list[i] == s)
      return true;
  }
  return false;
}

void Squarelist::clear()
{
  _no_of_elements = 0;
  _listindex = 0;
}

Square* Squarelist::operator[](int i) const
{
  if ((i < _no_of_elements) && (i >= 0))
    return (Square*) _list[i];
  else
  {
    std::cerr << "index out of bound error in Squarelist[]" << std::endl;
    return NULL;
  }
}
}

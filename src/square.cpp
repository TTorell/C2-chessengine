#include "square.hpp"

namespace C2_chess
{

//Square::Square(const Square& s) :
//    _position(s._position),
//    _moves(),
//    _threats(),
//    _protections()
//{
//  if (_piece)
//  {
//    delete _piece;
//    _piece = 0; // maybe not necessary
//  }
//  if (s._piece)
//    _piece = new Piece(s._piece);
//}

//Square& Square::operator=(const Square& s)
//{
//  if (_piece)
//  {
//    delete _piece;
//    _piece = 0;
//  }
//  if (s._piece)
//    _piece = new Piece(*s._piece);
//  return *this;
//}

std::ostream& Square::write_describing(std::ostream& os) const
{
  os << std::endl;
  if (_piece)
    os << *_piece;
  os << _position << std::endl;
  write_protections(os);
  write_threats(os);
  write_moves(os);
  return os;
}

std::ostream& Square::write_name(std::ostream& os) const
{
  if (_piece)
    os << *_piece;
  os << _position << "\n";
  return os;
}

std::ostream& Square::write_move_style(std::ostream& os) const
{
  os << _position;
  return os;
}

std::ostream& Square::write_threats(std::ostream& os) const
{
  os << "--- Threatened by: ---" << std::endl;
  Square* temp = _threats.first();
  while (temp)
  {
    temp->write_name(os);
    temp = _threats.next();
  }
  return os;
}

std::ostream& Square::write_protections(std::ostream& os) const
{
  os << "--- protected by: ---\n";
  Square* temp = _protections.first();
  while (temp)
  {
    temp->write_name(os);
    temp = _protections.next();
  }
  return os;
}

std::ostream& Square::write_moves(std::ostream& os) const
{
  os << "--- can move to: ---\n";
  Square* temp = _moves.first();
  while (temp)
  {
    temp->write_name(os);
    temp = _moves.next();
  }
  return os;
}
}

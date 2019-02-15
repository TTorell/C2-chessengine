#include "square.hpp"
#include "piece.hpp"
#include "chesstypes.hpp"

namespace C2_chess
{

using std::endl;

Square::Square() // Not available (private)
{
  _piece = 0;
  _colour = white; //Just to avoid code analysis problem
}

Square::Square(int file, int rank, col thiscolour, piecetype pt, col pc) :
    _position(file,
              rank),
    _colour(thiscolour),
    _moves(),
    _threats(),
    _protections()

{
  if (pt == Undefined)
    _piece = 0;
  else
    _piece = new Piece(pt,
                       pc);
}

Square::Square(const Square& s) :
    _position(s._position),
    _colour(s._colour),
    _moves(),
    _threats(),
    _protections()
{
  if (_piece)
  {
    delete _piece;
    _piece = 0; // maybe not necessary
  }
  if (s._piece)
    _piece = new Piece(*s._piece);
}

Square::~Square()
{
  if (_piece)
  {
    delete _piece;
    _piece = 0; // maybe not necessary
  }
}

Square& Square::operator=(const Square& s)
{
  _colour = s._colour;
  if (_piece)
  {
    delete _piece;
    _piece = 0;
  }
  if (s._piece)
    _piece = new Piece(*s._piece);
  return *this;
}

void Square::clear(bool delete_piece)
{
  if (delete_piece)
  {
    if (_piece)
    {
      delete (_piece);
      _piece = 0;
    }
  }
  _moves.clear();
  _threats.clear();
  _protections.clear();
}

ostream& Square::write_describing(ostream& os) const
{
  os << endl;
  if (_piece)
    os << *_piece;
  os << _position << endl;
  write_protections(os);
  write_threats(os);
  write_moves(os);
  return os;
}

ostream& Square::write_name(ostream& os) const
{
  if (_piece)
    os << *_piece;
  os << _position << "\n";
  return os;
}

ostream& Square::write_move_style(ostream& os) const
{
  os << _position;
  return os;
}

void Square::contain_piece(Piece* const newpiece)
{
  if (_piece)
    delete (_piece);
  _piece = newpiece;
}

bool Square::contains_piece(col c, piecetype pt) const
{
  if (_piece && _piece->is(c,
                           pt))
    return true;
  return false;
}

Piece* Square::release_piece()
{
  Piece* temp = _piece;
  _piece = 0;
  return temp;
}

void Square::remove_piece()
{
  if (_piece)
  {
    delete (_piece);
    _piece = 0;
  }
}

void Square::into_threat(Square* const newthreat)
{
  _threats.into(newthreat);
}

void Square::into_protection(Square* const newprotection)
{
  _protections.into(newprotection);
}

void Square::into_move(Square* const newprotection)
{
  _moves.into(newprotection);
}


void Square::out_threat(Square* const rubbish)
{
  _threats.out(rubbish);
}

void Square::out_protection(Square* const rubbish)
{
  _protections.out(rubbish);
}

void Square::out_move(Square* const rubbish)
{
  _moves.out(rubbish);
}

int Square::count_threats(col player) const
{
  int counter = 0;
  for (int i = 0; i < _threats.cardinal(); i++)
  {
    if (_threats[i]->_piece) // should always be true, but ...
    {
      if (_threats[i]->_piece->get_color() == player)
        counter++;
    }
  }
  return counter;
}

int Square::count_threats() const
{
  // Attention! if there's no piece on the square
  // There can be both black and white threats in the list.
  return _threats.cardinal();
}

int Square::count_protections() const
{
  //There are only protections if the Square contains a piece.
  return _protections.cardinal();
}

int Square::count_moves() const
{
  return _moves.cardinal();
}

void Square::clear_threats()
{
  _threats.clear();
}

void Square::clear_protections()
{
  _protections.clear();
}

void Square::clear_moves()
{
  _moves.clear();
}

ostream& Square::write_threats(ostream& os) const
{
  os << "--- Threatened by: ---" << endl;
  Square* temp = _threats.first();
  while (temp)
  {
    temp->write_name(os);
    temp = _threats.next();
  }
  return os;
}

ostream& Square::write_protections(ostream& os) const
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

ostream& Square::write_moves(ostream& os) const
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

bool Square::in_movelist(Square* const s) const
{
  bool temp = false;
  if (_moves.in_list(s))
    temp = true;
  return temp;
}

bool Square::same_file(Square* const s) const
{
  return _position.same_file(s->_position);
  //_position_file->is_included(compare_square);
}

bool Square::same_rank(Square* const s) const
{
  return _position.same_rank(s->_position);
  //return _rank->is_included(compare_square);
}

bool Square::same_diagonal(Square* const s) const
{
  return _position.same_diagonal(s->_position);
}

bool Square::contains(col c, piecetype pt)
{
  if (_piece && _piece->is(c, pt))
  {
    return true;
  }
  return false;
}

int Square::count_controls() const
{
  // We have to consider if the square holds a piece.
  // If there's no piece, the threats are mixed and there
  // are no protections.
  // If there is a piece, we have protections and the
  // threats are only from the other color.
  if (_piece)
  {
    if (_piece->get_color() == white)
      return (count_protections() - count_threats());
    else
      //piece is black
      return (count_threats() - count_protections());
  }
  // no piece (no protections only mixed threats)
  return (count_threats(white) - count_threats(black));
}
}

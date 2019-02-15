#include "piece.hpp"
#include "chesstypes.hpp"

namespace C2_chess
{

using std::endl;
using std::cerr;

Piece::Piece():_type(Undefined),_color(white)
{
}

Piece::Piece(piecetype t, col c):_type(t),_color(c)
{
}

Piece::Piece(const Piece& p):_type(p._type),_color(p._color)
{
}

Piece::~Piece()
{
}

bool Piece::operator==(const Piece& p) const
{
	return (p._type==_type && p._color==_color);
}

bool Piece::is(col c, piecetype pt) const
{
  return (_color == c && _type == pt);
}

bool Piece::is(piecetype pt) const
{
  return (_type == pt);
}

Piece& Piece::operator=(const Piece& p)
{
	_type = p._type;
	_color = p._color;
	return *this;
}

ostream& Piece::write_describing(ostream& os) const
{
	if (_color==black)
		os << "black ";
	if (_color==white)
		os << "white ";
	switch (_type)
	{
		case King: os << "King";
					  break;
		case Queen: os << "Queen";
						break;
		case Rook: os << "Rook";
					  break;
		case Bishop: os << "Bishop";
						 break;
		case Knight: os << "Knight";
						 break;
		case Pawn: os << "Pawn";
					  break;
		default: cerr << "Undefined piece type in Piece::write_describing" << endl;
	}
	return os;
}

ostream& Piece::write_diagram_style(ostream& os) const
{
    switch (_type)
    {
      case King:
        os << ((_color==white) ? "\u2654" : "\u265A");
        break;
      case Queen:
        os << ((_color==white) ? "\u2655":"\u265B");
        break;
      case Rook:
        os << ((_color==white) ? "\u2656":"\u265C");
        break;
      case Bishop:
        os << ((_color==white) ? "\u2657":"\u265D");
        break;
      case Knight:
        os << ((_color==white) ? "\u2658":"\u265E");
        break;
      case Pawn:
        os << ((_color==white) ? "\u2659":"\u265F");
        break;
      default: cerr << "Undefined piece type in Piece::write_diagram_style" << endl;
    }
    return os;
}

ostream& operator<<(ostream& os, const Piece& p)
{
	switch (p._type)
	{
		case King: os<<'K';
					  break;
		case Queen: os<<'Q';
						break;
		case Rook: os<<'R';
					  break;
		case Bishop: os<<'B';
						 break;
		case Knight: os<<'N';
						 break;
		case Pawn: os<<'P';
					  break;
            default: os << '?';
	}
	return os;
}
}

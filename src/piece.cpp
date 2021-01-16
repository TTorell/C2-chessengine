#include "piece.hpp"
#include "chesstypes.hpp"

namespace C2_chess
{

using std::endl;
using std::cerr;




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
        os << ((_color==white) ? u"\u2654" : u"\u265A");
        break;
      case Queen:
        os << ((_color==white) ? u"\u2655":u"\u265B");
        break;
      case Rook:
        os << ((_color==white) ? u"\u2656":u"\u265C");
        break;
      case Bishop:
        os << ((_color==white) ? u"\u2657":u"\u265D");
        break;
      case Knight:
        os << ((_color==white) ? u"\u2658":u"\u265E");
        break;
      case Pawn:
        os << ((_color==white) ? u"\u2659":u"\u265F");
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

#include "piece.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"

namespace C2_chess
{

std::ostream& Piece::write_describing(std::ostream& os) const
{
	if (_color==col::black)
		os << "black ";
	if (_color==col::white)
		os << "white ";
	switch (_type)
	{
		case piecetype::King: os << "King";
					  break;
		case piecetype::Queen: os << "Queen";
						break;
		case piecetype::Rook: os << "Rook";
					  break;
		case piecetype::Bishop: os << "Bishop";
						 break;
		case piecetype::Knight: os << "Knight";
						 break;
		case piecetype::Pawn: os << "Pawn";
					  break;
		default: std::cerr << "Undefined piece type in Piece::write_describing" << std::endl;
	}
	return os;
}

std::ostream& Piece::write_diagram_style(std::ostream& os) const
{
    switch (_type)
    {
      case piecetype::King:
        os << ((_color==col::white) ? ("\u2654") : ("\u265A"));
        break;
      case piecetype::Queen:
        os << ((_color== col::white) ? ("\u2655"): ("\u265B"));
        break;
      case piecetype::Rook:
        os << ((_color== col::white) ? ("\u2656"): ("\u265C"));
        break;
      case piecetype::Bishop:
        os << ((_color== col::white) ? ("\u2657"): ("\u265D"));
        break;
      case piecetype::Knight:
        os << ((_color== col::white) ? ("\u2658"): ("\u265E"));
        break;
      case piecetype::Pawn:
        os << ((_color== col::white) ? ("\u2659"): ("\u265F"));
        break;
      default: std::cerr << ("Undefined piece type in Piece::write_diagram_style") << std::endl;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Piece& p)
{
	switch (p._type)
	{
	    case piecetype::King: os<<'K';
					  break;
		case piecetype::Queen: os<<'Q';
						break;
		case piecetype::Rook: os<<'R';
					  break;
		case piecetype::Bishop: os<<'B';
						 break;
		case piecetype::Knight: os<<'N';
						 break;
		case piecetype::Pawn: os<<'P';
					  break;
        default: os << '?';
	}
	return os;
}
}

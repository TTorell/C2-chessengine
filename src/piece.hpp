#ifndef _PIECE
#define _PIECE

#include <iostream>
#include "chesstypes.hpp"

namespace C2_chess
{

using std::ostream;

class Piece
{
	protected:
		piecetype _type;
		col _color;
	private:
		Piece();
	public:
		Piece(piecetype t,col c);
		Piece(const Piece& p);
		~Piece();
		bool operator==(const Piece& p) const;
		bool is(col c, piecetype pt) const;
        bool is(piecetype pt) const;
		Piece& operator=(const Piece& p);
		piecetype get_type() const {return _type;};
		col get_color() const {return _color;};
		void set_type(piecetype t) {_type=t;};
		void set_color(col c) {_color=c;};
		ostream& write_describing(ostream& os) const;
        ostream& write_diagram_style(ostream& os) const;
      friend ostream& operator<<(ostream& os, const Piece& m);
};
}
#endif

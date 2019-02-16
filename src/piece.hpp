#ifndef _PIECE
#define _PIECE

#include <iostream>
#include "chesstypes.hpp"

namespace C2_chess
{

using std::ostream;

class Piece {
  protected:
    piecetype _type;
    col _color;

  private:
    Piece() :
        _type(Undefined), _color(white)
    {
    }

  public:
    Piece(piecetype t, col c) :
        _type(t), _color(c)
    {
    }
    Piece(const Piece& p) :
        _type(p._type), _color(p._color)
    {
    }
    ~Piece()
    {
    }
    bool operator==(const Piece& p) const
    {
      return (p._type == _type && p._color == _color);
    }
    Piece& operator=(const Piece& p)
    {
      _type = p._type;
      _color = p._color;
      return *this;
    }
    bool is(col c, piecetype pt) const
    {
      return (_color == c && _type == pt);
    }
    bool is(piecetype pt) const
    {
      return (_type == pt);
    }
    bool is(col c) const
    {
      return (_color == c);
    }
    piecetype get_type() const
    {
      return _type;
    }
    col get_color() const
    {
      return _color;
    }
    void set_type(piecetype t)
    {
      _type = t;
    }
    void set_color(col c)
    {
      _color = c;
    }
    ostream& write_describing(ostream& os) const;
    ostream& write_diagram_style(ostream& os) const;
    friend ostream& operator<<(ostream& os, const Piece& m);
};
}
#endif

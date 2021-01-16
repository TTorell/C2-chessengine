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
    Piece(piecetype t, col color) :
        _type(t), _color(color)
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
    bool is(col color, piecetype pt) const
    {
      return (_color == color && _type == pt);
    }
    bool is(piecetype pt) const
    {
      return (_type == pt);
    }
    bool is(col color) const
    {
      return (_color == color);
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
    void set_color(col color)
    {
      _color = color;
    }
    ostream& write_describing(ostream& os) const;
    ostream& write_diagram_style(ostream& os) const;
    friend ostream& operator<<(ostream& os, const Piece& m);
};
}
#endif

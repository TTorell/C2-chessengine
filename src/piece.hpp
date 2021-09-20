#ifndef _PIECE
#define _PIECE

#include <iostream>
#include "chesstypes.hpp"

namespace C2_chess
{

class Piece {
  protected:
    piecetype _type;
    col _color;

  private:
    Piece() :
        _type(piecetype::Undefined), _color(col::white)
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
    std::ostream& write_describing(std::ostream& os) const;
    std::ostream& write_diagram_style(std::ostream& os) const;
    friend std::ostream& operator<<(std::ostream& os, const Piece& m);
};
}
#endif

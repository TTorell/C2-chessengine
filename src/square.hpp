#ifndef _SQUARE
#define _SQUARE

#include <iostream>
#include "squarelist.hpp"
#include "position.hpp"
#include "piece.hpp"
#include "chesstypes.hpp"

namespace C2_chess
{

class Square {
  protected:
    Piece* _piece;
    Position _position;
    Squarelist _moves;
    Squarelist _threats;
    Squarelist _protections;

  private:
    Square() :
        _piece(0)
    {
    }

  public:
    Square(int file, int rank) :
        _piece(0), _position(file, rank), _moves(), _threats(), _protections()
    {
    }
    //Square(const Square& s);
    ~Square()
    {
      if (_piece)
      {
        delete _piece;
      }
    }
    //Square& operator=(const Square& s);
    Position get_position() const
    {
      return _position;
    }
    int get_fileindex() const
    {
      return _position.get_file();
    }
    int get_rankindex() const
    {
      return _position.get_rank();
    }
    void clear(bool delete_piece = true)
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
    void contain_piece(Piece* newpiece)
    {
      if (_piece)
        delete (_piece);
      _piece = newpiece;
    }
    bool contains_piece(col color, piecetype pt) const
    {
      if (_piece && _piece->is(color, pt))
        return true;
      return false;
    }
    Piece* release_piece()
    {
      Piece* temp = _piece;
      _piece = 0;
      return temp;
    }
    void remove_piece()
    {
      if (_piece)
      {
        delete (_piece);
        _piece = 0;
      }
    }
    Piece* get_piece() const
    {
      return _piece;
    }
    void into_threat(Square* const newthreat)
    {
      _threats.into(newthreat);
    }
    void into_protection(Square* const newprotection)
    {
      _protections.into(newprotection);
    }
    void into_move(Square* const newmove)
    {
      _moves.into(newmove);
    }
    void out_threat(Square* const rubbish)
    {
      _threats.out(rubbish);
    }
    void out_protection(Square* const rubbish)
    {
      _protections.out(rubbish);
    }
    void out_move(Square* const rubbish)
    {
      _moves.out(rubbish);
    }
    int count_threats() const
    {
      // Attention! if there's no piece on the square
      // There can be both black and white threats in the list.
      return _threats.cardinal();
    }
    int count_threats(col color) const
    {
      int counter = 0;
      for (int i = 0; i < _threats.cardinal(); i++)
      {
        if (_threats[i]->_piece) // should always be true, but ...
        {
          if (_threats[i]->_piece->is(color))
            counter++;
        }
      }
      return counter;
    }
    int count_protections() const
    {
      //There are only protections if the Square contains a piece.
      return _protections.cardinal();
    }

    int count_moves() const
    {
      return _moves.cardinal();
    }
    void clear_threats()
    {
      _threats.clear();
    }
    void clear_protections()
    {
      _protections.clear();
    }
    void clear_moves()
    {
      _moves.clear();
    }
    Square* first_threat() const
    {
      return _threats.first();
    }
    Square* first_protection() const
    {
      return _protections.first();
    }
    Square* first_move() const
    {
      return _moves.first();
    }
    Square* next_move() const
    {
      return _moves.next();
    }
    Square* next_protection() const
    {
      return _protections.next();
    }
    Square* next_threat() const
    {
      return _threats.next();
    }
    bool in_movelist(Square* const s) const
    {
      if (_moves.in_list(s))
        return true;
      return false;
    }
    bool same_file(Square* s) const
    {
      return _position.same_file(s->_position);
    }
    bool same_rank(Square*s) const
    {
      return _position.same_rank(s->_position);
    }
    bool same_diagonal(Square* s) const
    {
      return _position.same_diagonal(s->_position);
    }
    bool contains(col color, piecetype pt)
    {
      if (_piece && _piece->is(color, pt))
        return true;
      return false;
    }
    int count_controls() const // For evaluation of position
    {
      // We have to consider if the square holds a piece.
      // If there's no piece, the threats are mixed and there
      // are no protections.
      // If there is a piece, we have protections and the
      // threats are only from the other color.
      if (_piece)
      {
        if (_piece->is(col::white))
          return (count_protections() - count_threats());
        else
          return (count_threats() - count_protections());
      }
      // no piece (no protections only mixed threats)
      return (count_threats(col::white) - count_threats(col::black));
    }
    operator const Position&() const
    {
      return _position;
    }
    ostream& write_threats(ostream& os) const;
    ostream& write_protections(ostream& os) const;
    ostream& write_moves(ostream& os) const;
    ostream& write_describing(ostream& os) const;
    ostream& write_name(ostream& os) const;
    ostream& write_move_style(ostream& os) const;
};
}
#endif


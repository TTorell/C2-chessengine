#ifndef _MOVE
#define _MOVE

#include "position.hpp"
#include "chesstypes.hpp"

using namespace std;

namespace C2_chess
{

class Move
{
  protected:
    Position _from;
    Position _to;
    piecetype _piece_type;
    bool _take;
    piecetype _target_piece_type;
    bool _en_passant;
    bool _promotion;
    piecetype _promotion_piece_type;
    bool _check;
    bool _mate;
    bool _stalemate;

  public:
    Move();

    Move(const Square* from,
         const Square* to,
         piecetype piece_type = piecetype::Pawn,
         bool take = false,
         piecetype target_pt = piecetype::Pawn,
         bool ep = false,
         bool promotion = false,
         piecetype promotion_pt = piecetype::Undefined,
         bool check = false,
         bool mate = false,
         bool stalemate = false);

    Move(const Position& from,
         const Position& to,
         piecetype piece_type = piecetype::Pawn,
         bool take = false,
         piecetype target_pt = piecetype::Pawn,
         bool ep = false,
         bool promotion = false,
         piecetype promotion_pt = piecetype::Undefined,
         bool check = false,
         bool mate = false,
         bool stalemate = false);

    Move(const Move& m);

    Move(Move* const m);

    ~Move();

    bool operator==(const Move& m);
    Move& operator=(const Move& m);
    Position get_from() const
    {
      return _from;
    }
    int get_from_rankindex() const
    {
      return _from.get_rank();
    }
    int get_from_fileindex() const
    {
      return _from.get_file();
    }
    int get_to_fileindex() const
    {
      return _to.get_file();
    }
    Position get_to() const
    {
      return _to;
    }
    piecetype get_piece_type() const
    {
      return _piece_type;
    }
    bool get_take() const
    {
      return _take;
    }
    piecetype get_target_piece_type() const
    {
      return _target_piece_type;
    }
    bool get_en_passant() const
    {
      return _en_passant;
    }
    bool get_promotion() const
    {
      return _promotion;
    }
    piecetype get_promotion_piece_type() const
    {
      return _promotion_piece_type;
    }
    bool get_check() const
    {
      return _check;
    }
    bool get_mate() const
    {
      return _mate;
    }
    bool get_stalemate() const
    {
      return _stalemate;
    }
    void set_from(const Position& p)
    {
      _from = p;
    }
    void set_to(const Position& p)
    {
      _to = p;
    }
    void set_piece_type(piecetype pt)
    {
      _piece_type = pt;
    }
    void set_take(bool t)
    {
      _take = t;
    }
    void set_target_piece_type(piecetype pt)
    {
      _target_piece_type = pt;
    }
    void set_en_passant(bool ep)
    {
      _en_passant = ep;
    }
    void set_promotion(bool promotion)
    {
      _promotion = promotion;
    }
    void set_promotion_piece_type(const piecetype pt)
    {
      _promotion_piece_type = pt;
    }
    void set_check(bool color)
    {
      _check = color;
    }
    void set_mate(bool m)
    {
      _mate = m;
    }
    void set_stalemate(bool m)
    {
      _stalemate = m;
    }
    string bestmove_engine_style() const;
    friend ostream& operator<<(ostream& os, const Move& m);
};
}
#endif


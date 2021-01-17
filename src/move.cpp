#include "move.hpp"
#include "square.hpp"
#include "piece.hpp"
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include <sstream>

namespace C2_chess
{

    Move::Move() :
        _from(f, 6), _to(g, 8), _piece_type(piecetype::Knight), _take(false), _target_piece_type(piecetype::Pawn), _en_passant(false), _promotion(false), _promotion_piece_type(piecetype::Undefined), _check(false),
    _mate(false), _stalemate(false)
{
}

Move::Move(const Square* from, const Square* to, piecetype pt, bool take, piecetype target_pt, bool ep, bool promotion, piecetype promotion_pt, bool check, bool mate,
           bool stalemate) :
    _from(from->get_position()), _to(to->get_position()), _piece_type(pt), _take(take), _target_piece_type(target_pt), _en_passant(ep), _promotion(promotion),
    _promotion_piece_type(promotion_pt), _check(check), _mate(mate), _stalemate(stalemate)
{
  Piece* p = from->get_piece();
  require_m(p,
  __FILE__,
            __func__,
            __LINE__,
            *this);
  _piece_type = p->get_type();
  Piece* p2 = to->get_piece();
  if (p2)
  {
    require_m(p2->get_color() != p->get_color(),
    __FILE__,
              __func__,
              __LINE__,
              *this);
    _take = true;
    _target_piece_type = p2->get_type();
    _en_passant = false;
    if (_piece_type == piecetype::Pawn)
      if (p->get_color() == col::white ? _from.get_rank() == 7 : _from.get_rank() == 2)
        _promotion = true;
  }
  else // No piece on targetsquare
  {
    _target_piece_type = piecetype::Undefined;
    int file_diff = abs(_to.get_file() - _from.get_file());
    int rank_diff = abs(_to.get_rank() - _from.get_rank());
    switch (_piece_type)
    {
      case piecetype::Pawn:
      {
        require_m(rank_diff <= 2,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        if (rank_diff == 2)
        {
          require_m(file_diff == 0,
          __FILE__,
                    __func__,
                    __LINE__,
                    *this);
          require_m(p->get_color() == col::white ? _from.get_rank() == 2 : _from.get_rank() == 7,
          __FILE__,
                    __func__,
                    __LINE__,
                    *this);
          _take = false;
          _promotion = false;
          _promotion_piece_type = piecetype::Undefined;
        }
        else
        {
          require_m(rank_diff == 1 && file_diff <= 1,
          __FILE__,
                    __func__,
                    __LINE__,
                    *this);
          if (p->get_color() == col::white ? _from.get_rank() == 7 : _from.get_rank() == 2)
            _promotion = true;
          if (file_diff == 0)
          {
            _take = false;
            _en_passant = false;
            _target_piece_type = piecetype::Pawn;
          }
          else //rank_diff==1 and file_diff==1 and not a take
          {
            require_m(p->get_color() == col::white ? _from.get_rank() == 5 : _from.get_rank() == 4,
            __FILE__,
                      __func__,
                      __LINE__,
                      *this);
            _en_passant = true;
            _take = true;
            _target_piece_type = piecetype::Pawn;
            _promotion = false;
          }
        }
        break;
      }
      case piecetype::Rook:
      {
        require_m(file_diff == 0 || rank_diff == 0,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        _promotion = false;
        _promotion_piece_type = piecetype::Undefined;
        _en_passant = false;
        break;
      }
      case piecetype::Knight:
      {
        require_m(file_diff <= 2 && rank_diff <= 2 && file_diff >= 1 && rank_diff >= 1,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        require_m(abs(file_diff - rank_diff) == 1,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        _promotion = false;
        _promotion_piece_type = piecetype::Undefined;
        _en_passant = false;
        break;
      }
      case piecetype::Bishop:
      {
        require_m(file_diff == rank_diff,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        _promotion = false;
        _promotion_piece_type = piecetype::Undefined;
        _en_passant = false;
        break;
      }
      case piecetype::Queen:
      {
        require_m(file_diff == 0 || rank_diff == 0 || file_diff == rank_diff,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        _promotion = false;
        _promotion_piece_type = piecetype::Undefined;
        _en_passant = false;
        break;
      }
      case piecetype::King:
      {
        require_m(rank_diff <= 1 && file_diff <= 2,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
        if (file_diff == 2) // Rooking
        {
          require_m(rank_diff == 0,
          __FILE__,
                    __func__,
                    __LINE__,
                    *this);
          require_m(from->get_position().get_file() == e,
          __FILE__,
                    __func__,
                    __LINE__,
                    *this);
          if (p->get_color() == col::white)
            require_m(from->get_position().get_rank() == 1,
            __FILE__,
                      __func__,
                      __LINE__,
                      *this);
          else
            require_m(from->get_position().get_rank() == 8,
            __FILE__,
                      __func__,
                      __LINE__,
                      *this);
        }
        _promotion = false;
        _promotion_piece_type = piecetype::Undefined;
        _en_passant = false;
        break;
      }
      default:
        require_m(1 == 2,
        __FILE__,
                  __func__,
                  __LINE__,
                  *this);
    } // end of switch
  }
}

Move::Move(const Position& from, const Position& to, piecetype pt, bool take, piecetype target_pt, bool ep, bool promotion, piecetype promotion_pt, bool is_check, bool is_mate,
           bool is_stalemate) :
    _from(from), _to(to), _piece_type(pt), _take(take), _target_piece_type(target_pt), _en_passant(ep), _promotion(promotion), _promotion_piece_type(promotion_pt),
    _check(is_check), _mate(is_mate), _stalemate(is_stalemate)
{
  //int file_diff=abs(_to.get_file()-_from.get_file());
  //int rank_diff=abs(_to.get_file()-_from.get_file());
}

Move::Move(const Move& m) :
    _from(m._from), _to(m._to), _piece_type(m._piece_type), _take(m._take), _target_piece_type(m._target_piece_type), _en_passant(m._en_passant), _promotion(m._promotion),
    _promotion_piece_type(m._promotion_piece_type), _check(m._check), _mate(m._mate), _stalemate(m._stalemate)
{
}

Move::Move(Move* const m) :
    _from((*m)._from), _to((*m)._to), _piece_type((*m)._piece_type), _take((*m)._take), _target_piece_type((*m)._target_piece_type), _en_passant((*m)._en_passant),
    _promotion((*m)._promotion), _promotion_piece_type((*m)._promotion_piece_type), _check((*m)._check), _mate((*m)._mate), _stalemate((*m)._stalemate)
{
}

Move::~Move()
{
}

bool Move::operator==(const Move& m)
{
  return (m._from == _from && m._to == _to);
}

Move& Move::operator=(const Move& m)
{
  _from = m._from;
  _to = m._to;
  _piece_type = m._piece_type;
  _take = m._take;
  _target_piece_type = m._target_piece_type;
  _en_passant = m._en_passant;
  _promotion = m._promotion;
  _promotion_piece_type = m._promotion_piece_type;
  _check = m._check;
  _mate = m._mate;
  _stalemate = m._stalemate;
  return *this;
}

string Move::bestmove_engine_style()
{
  stringstream ss;
  ss << "bestmove " << _from << _to;
  return ss.str();
}

ostream& operator<<(ostream& os, const Move& m)
{
  switch (m._piece_type)
  {
    case piecetype::King:
    {
      if ((m._to.get_file() - m._from.get_file()) == 2 || (m._to.get_file() - m._from.get_file()) == -2)
      {
        if (m._to.get_file() == g)
          os << "0-0";
        else
          os << "0-0-0";
        return os;
      }
      os << "K";
      break;
    }
    case piecetype::Queen:
      os << "Q";
      break;
    case piecetype::Rook:
      os << "R";
      break;
    case piecetype::Bishop:
      os << "B";
      break;
    case piecetype::Knight:
      os << "N";
      break;
    default:
      break;
  }
  os << m._from;
  if (m._take)
    os << "x";
  else
    os << "-";
  os << m._to;
  if (m._en_passant)
    os << " " << "e.p.";
  switch (m._promotion_piece_type)
  {
    case piecetype::Queen:
      os << "Q";
      break;
    case piecetype::Rook:
      os << "R";
      break;
    case piecetype::Bishop:
      os << "B";
      break;
    case piecetype::Knight:
      os << "N";
      break;
    default:
      break;
  }
  if (m._check)
    os << "+";
  if (m._mate)
    os << " mate";
  if (m._stalemate)
    os << " stalemate";
  return os;
}
}

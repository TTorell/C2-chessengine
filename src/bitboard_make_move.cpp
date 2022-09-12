/*
 * Bitboard_make_move.cpp
 *
 *  Created on: Nov 3, 2021
 *      Author: torsten
 */

#include "bitboard.hpp"
#include "chesstypes.hpp"
#include "game_history.hpp"
#include "transposition_table.hpp"

namespace C2_chess
{

Transposition_table Bitboard::transposition_table;

inline void Bitboard::add_promotion_piece(Piecetype p_type)
{
  switch (p_type)
  {
    case Piecetype::Queen:
      (_side_to_move == Color::White) ? _material_diff += 8.0 : _material_diff -= 8.0;
      break;
    case Piecetype::Rook:
      (_side_to_move == Color::White) ? _material_diff += 4.0 : _material_diff -= 4.0;
      break;
    case Piecetype::Knight:
      (_side_to_move == Color::White) ? _material_diff += 2.0 : _material_diff -= 2.0;
      break;
    case Piecetype::Bishop:
      (_side_to_move == Color::White) ? _material_diff += 2.0 : _material_diff -= 2.0;
      break;
    default:
      ;
  }
}

inline void Bitboard::touch_piece(uint64_t square)
{
  if (_side_to_move == Color::White)
  {
    if (_white_pieces.Queens & square)
      _white_pieces.Queens ^= square;
    else if (_white_pieces.Pawns & square)
      _white_pieces.Pawns ^= square;
    else if (_white_pieces.King & square)
      _white_pieces.King ^= square;
    else if (_white_pieces.Rooks & square)
      _white_pieces.Rooks ^= square;
    else if (_white_pieces.Bishops & square)
      _white_pieces.Bishops ^= square;
    else if (_white_pieces.Knights & square)
      _white_pieces.Knights ^= square;
  }
  else
  {
    if (_black_pieces.Queens & square)
      _black_pieces.Queens ^= square;
    else if (_black_pieces.Pawns & square)
      _black_pieces.Pawns ^= square;
    else if (_black_pieces.King & square)
      _black_pieces.King ^= square;
    else if (_black_pieces.Rooks & square)
      _black_pieces.Rooks ^= square;
    else if (_black_pieces.Bishops & square)
      _black_pieces.Bishops ^= square;
    else if (_black_pieces.Knights & square)
      _black_pieces.Knights ^= square;
  }
}

Piecetype Bitboard::get_piece_type(uint64_t square) const
{
  assert(square & _all_pieces);
  if ((_white_pieces.Pawns & square) || (_black_pieces.Pawns & square))
  {
    return Piecetype::Pawn;
  }
  else if ((_white_pieces.Queens & square) || (_black_pieces.Queens & square))
  {
    return Piecetype::Queen;
  }
  else if ((_white_pieces.Rooks & square) || (_black_pieces.Rooks & square))
  {
    return Piecetype::Rook;
  }
  else if ((_white_pieces.Bishops & square) || (_black_pieces.Bishops & square))
  {
    return Piecetype::Bishop;
  }
  else if ((_white_pieces.Knights & square) || (_black_pieces.Knights & square))
  {
    return Piecetype::Knight;
  }
  else if ((_white_pieces.King & square) || (_black_pieces.King & square))
  {
    return Piecetype::King;
  }
  return Piecetype::Undefined;
}


inline void Bitboard::remove_other_piece(uint64_t square)
{
  Piecetype pt = Piecetype::Undefined;
  if (_side_to_move == Color::White)
  {
    if (_black_pieces.Queens & square)
    {
      _black_pieces.Queens ^= square, _material_diff += 9, pt = Piecetype::Queen;
    }
    else if (_black_pieces.Pawns & square)
    {
      _black_pieces.Pawns ^= square, _material_diff += 1, pt = Piecetype::Pawn;
    }
    else if (_black_pieces.Rooks & square)
    {
      _black_pieces.Rooks ^= square, _material_diff += 5, pt = Piecetype::Rook;
      // update castling rights if needed
      if (square & h8_square)
        remove_castling_right(castling_right_BK);
      else if (square & a8_square)
        remove_castling_right(castling_right_BQ);
    }
    else if (_black_pieces.Bishops & square)
      _black_pieces.Bishops ^= square, _material_diff += 3, pt = Piecetype::Bishop;
    else if (_black_pieces.Knights & square)
      _black_pieces.Knights ^= square, _material_diff += 3, pt = Piecetype::Knight;
    update_hash_tag(square, Color::Black, pt);
  }
  else
  {
    if (_white_pieces.Queens & square)
      _white_pieces.Queens ^= square, _material_diff -= 9, pt = Piecetype::Queen;
    else if (_white_pieces.Pawns & square)
    {
      _white_pieces.Pawns ^= square, _material_diff -= 1, pt = Piecetype::Pawn;
    }
    else if (_white_pieces.Rooks & square)
    {
      _white_pieces.Rooks ^= square, _material_diff -= 5, pt = Piecetype::Rook;
      // update castling rights if needed
      if (square & h1_square)
        remove_castling_right(castling_right_WK);
      else if (square & a1_square)
        remove_castling_right(castling_right_WQ);
    }
    else if (_white_pieces.Bishops & square)
      _white_pieces.Bishops ^= square, _material_diff -= 3, pt = Piecetype::Bishop;
    else if (_white_pieces.Knights & square)
      _white_pieces.Knights ^= square, _material_diff -= 3, pt = Piecetype::Knight;
    assert(pt != Piecetype::Undefined);
    update_hash_tag(square, Color::White, pt);
  }
}

inline void Bitboard::place_piece(Piecetype p_type, uint64_t square)
{
  switch (p_type)
  {
    case Piecetype::Pawn:
      (_side_to_move == Color::White) ? _white_pieces.Pawns |= square : _black_pieces.Pawns |= square;
      break;
    case Piecetype::Queen:
      (_side_to_move == Color::White) ? _white_pieces.Queens |= square : _black_pieces.Queens |= square;
      break;
    case Piecetype::King:
      (_side_to_move == Color::White) ? _white_pieces.King |= square : _black_pieces.King |= square;
      break;
    case Piecetype::Rook:
      (_side_to_move == Color::White) ? _white_pieces.Rooks |= square : _black_pieces.Rooks |= square;
      break;
    case Piecetype::Knight:
      (_side_to_move == Color::White) ? _white_pieces.Knights |= square : _black_pieces.Knights |= square;
      break;
    case Piecetype::Bishop:
      (_side_to_move == Color::White) ? _white_pieces.Bishops |= square : _black_pieces.Bishops |= square;
      break;
    default:
      ;
  }
}

// Move the piece and update the hash_tag.
inline void Bitboard::move_piece(uint64_t from_square,
                                 uint64_t to_square,
                                 Piecetype p_type)
{
  uint64_t* p = nullptr;
  switch (p_type)
  {
    case Piecetype::Pawn:
      p = (_side_to_move == Color::White) ? &_white_pieces.Pawns : &_black_pieces.Pawns;
      break;
    case Piecetype::Queen:
      p = (_side_to_move == Color::White) ? &_white_pieces.Queens : &_black_pieces.Queens;
      break;
    case Piecetype::King:
      p = (_side_to_move == Color::White) ? &_white_pieces.King : &_black_pieces.King;
      break;
    case Piecetype::Rook:
      p = (_side_to_move == Color::White) ? &_white_pieces.Rooks : &_black_pieces.Rooks;
      break;
    case Piecetype::Knight:
      p = (_side_to_move == Color::White) ? &_white_pieces.Knights : &_black_pieces.Knights;
      break;
    case Piecetype::Bishop:
      p = (_side_to_move == Color::White) ? &_white_pieces.Bishops : &_black_pieces.Bishops;
      break;
    default:
      ;
  }
  (*p) ^= (from_square | to_square);
  update_hash_tag(from_square, to_square, _side_to_move, p_type);
}

// Preconditions: Remove One castling_right at the time
inline void Bitboard::remove_castling_right(uint8_t cr)
{
  if (_castling_rights & cr)
  {
    _castling_rights ^= cr;
    _hash_tag ^= transposition_table._castling_rights[cr];
  }
}

inline void Bitboard::clear_ep_square()
{
  _hash_tag ^= transposition_table._en_passant_file[file_idx(_ep_square)];
  _ep_square = zero;
}

inline void Bitboard::set_ep_square(uint64_t ep_square)
{
  _ep_square = ep_square;
  _hash_tag ^= transposition_table._en_passant_file[file_idx(_ep_square)];
}

// Set a "unique" hash tag for the position after
// adding or removing one piece from a square.
void Bitboard::update_hash_tag(uint64_t square, Color p_color, Piecetype p_type)
{
  _hash_tag ^= transposition_table._random_table[bit_idx(square)][index(p_color)][index(p_type)];
}

// Set a "unique" hash tag for the position after
// removing one piece from a square and putting
// the same piece on an empty square.
inline void Bitboard::update_hash_tag(uint64_t square1, uint64_t square2, Color p_color, Piecetype p_type)
{
  _hash_tag ^= (transposition_table._random_table[bit_idx(square1)][index(p_color)][index(p_type)] ^
                transposition_table._random_table[bit_idx(square2)][index(p_color)][index(p_type)]);
}

void Bitboard::update_side_to_move()
{
  _side_to_move = other_color(_side_to_move);
  if (_side_to_move == Color::White)
  {
    _own = &_white_pieces;
    _other = &_black_pieces;
  }
  else
  {
    _own = &_black_pieces;
    _other = &_white_pieces;
  }
  _hash_tag ^= transposition_table._black_to_move;
}

inline void Bitboard::update_state_after_king_move(const Bitmove& m)
{
  uint64_t from_square = m.from();
  uint64_t to_square = m.to();

  if (from_square & e1_square)
  {
    remove_castling_right(castling_right_WK);
    remove_castling_right(castling_right_WQ);
  }
  else if (from_square & e8_square)
  {
    remove_castling_right(castling_right_BK);
    remove_castling_right(castling_right_BQ);
  }
  // Move the rook if it's actually a castling move.
  if (m.properties() & move_props_castling)
  {
    if (from_square > to_square)
    {
      // Castling king side.
      if (_side_to_move == Color::White)
      {
        _white_pieces.Rooks ^= (h1_square | f1_square);
        update_hash_tag(h1_square, f1_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::White)] = true;
      }
      else
      {
        _black_pieces.Rooks ^= (h8_square | f8_square);
        update_hash_tag(h8_square, f8_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::Black)] = true;
     }
    }
    else
    {
      // Castling queen side.
      if (_side_to_move == Color::White)
      {
        _white_pieces.Rooks ^= (a1_square | d1_square);
        update_hash_tag(a1_square, d1_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::White)] = true;
      }
      else
      {
        _black_pieces.Rooks ^= (a8_square | d8_square);
        update_hash_tag(a8_square, d8_square, _side_to_move, Piecetype::Rook);
        _has_castled[index(Color::Black)] = true;
      }
    }
  }
}

void Bitboard::make_move(uint8_t i, Gentype gt, bool add_to_history)
{
  assert(i < _movelist.size());
  make_move(_movelist[i], gt, add_to_history);
}

// The move must be valid, but doesn't have to be in _movelist.
// _movelist may be empty.
void Bitboard::make_move(const Bitmove& m, Gentype gt, bool add_to_history)
{
  assert((_own->pieces & _other->pieces) == zero);
  uint64_t to_square = m.to();
  uint64_t from_square = m.from();

//  std::cout << "make_move: " << m << std::endl << to_binary_board(from_square) << std::endl << to_binary_board(to_square) << std::endl <<
//      to_binary_board(_other->pieces) << std::endl;

  // Clear _ep_square
  uint64_t tmp_ep_square = _ep_square;
  if (_ep_square)
    clear_ep_square();

// Remove possible piece of other color on to_square and
// Then make the move (updates hashtag)
  if (to_square & _other->pieces)
    remove_other_piece(to_square);
  move_piece(from_square, to_square, m.piece_type());

// OK we have moved the piece, now we must
// look at some special cases.
  if (m.piece_type() == Piecetype::King)
  {
    update_state_after_king_move(m);
  }
  else if (m.piece_type() == Piecetype::Rook)
  {
    // Clear castling rights for one side if applicable.
    if (to_square & _white_pieces.Rooks)
    {
      if (from_square & a1_square)
        remove_castling_right(castling_right_WQ);
      else if (from_square & h1_square)
        remove_castling_right(castling_right_WK);
    }
    else
    {
      // Black rook
      if (from_square & a8_square)
        remove_castling_right(castling_right_BQ);
      else if (from_square & h8_square)
        remove_castling_right(castling_right_BK);
    }
  }
  else if (m.piece_type() == Piecetype::Pawn)
  {
    if (m.properties() & move_props_promotion)
    {
      // Remove the pawn from promotion square
      // subtract 1 from the normal piece-values
      // because the pawn disappears from the board.
      (_side_to_move == Color::White) ? _white_pieces.Pawns ^= to_square : _black_pieces.Pawns ^= to_square;
      update_hash_tag(to_square, other_color(_side_to_move), Piecetype::Pawn);
      switch (m.promotion_piece_type())
      {
        case Piecetype::Queen:
          (_side_to_move == Color::White) ? (_material_diff += 8.0, _white_pieces.Queens |= to_square) :
                                         (_material_diff -= 8.0, _black_pieces.Queens |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Queen);
          break;
        case Piecetype::Rook:
          (_side_to_move == Color::White) ? (_material_diff += 4.0, _white_pieces.Rooks |= to_square) :
                                         (_material_diff -= 4.0, _black_pieces.Rooks |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Rook);
          break;
        case Piecetype::Knight:
          (_side_to_move == Color::White) ? (_material_diff += 2.0, _white_pieces.Knights |= to_square) :
                                         (_material_diff -= 2.0, _black_pieces.Knights |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Knight);
          break;
        case Piecetype::Bishop:
          (_side_to_move == Color::White) ? (_material_diff += 2.0, _white_pieces.Bishops |= to_square) :
                                         (_material_diff -= 2.0, _black_pieces.Bishops |= to_square);
          update_hash_tag(to_square, _side_to_move, Piecetype::Bishop);
          break;
        default:
          ;
      }
    }
    else if (m.properties() & move_props_en_passant)
    {
      // Remove the pawn taken e.p.
      if (_side_to_move == Color::White)
      {
        _black_pieces.Pawns ^= tmp_ep_square << 8, _material_diff += 1.0;
        update_hash_tag(tmp_ep_square << 8, Color::Black, Piecetype::Pawn);
      }
      else
      {
        _white_pieces.Pawns ^= tmp_ep_square >> 8, _material_diff -= 1.0;
        update_hash_tag(tmp_ep_square >> 8, Color::Black, Piecetype::Pawn);
      }
      update_hash_tag(to_square, other_color(_side_to_move), Piecetype::Pawn);
    }
    else if ((_side_to_move == Color::White) && (from_square & row_2) && (to_square & row_4))
    {
      // Check if there is a pawn of other color alongside to_square.
      if (((to_square & not_a_file) && ((to_square << 1) & _other->Pawns)) ||
          ((to_square & not_h_file) && ((to_square >> 1) & _other->Pawns)))
        set_ep_square(to_square << 8);
    }
    else if ((_side_to_move == Color::Black) && (from_square & row_7) && (to_square & row_5))
    {
      if (((to_square & not_a_file) && ((to_square << 1) & _other->Pawns)) ||
          ((to_square & not_h_file) && ((to_square >> 1) & _other->Pawns)))
        set_ep_square(to_square >> 8);
    }
  }
  // Set up the board for other player:
  _last_move = m;
  update_side_to_move();
  if (_side_to_move == Color::White)
    _move_number++;
  if (square_is_threatened(_own->King, false))
    _last_move.add_property(move_props_check);
  // add_position to game_history
  if (add_to_history)
  {
    history.add_position(_hash_tag);
  }
  update_half_move_counter();
  // TODO: Must be possible to change to gentype::captures.
  find_legal_moves(gt);
}

}
// namespace C2_chess


/*
 * Bitboard_make_move.cpp
 *
 *  Created on: Nov 3, 2021
 *      Author: torsten
 */


#include "bitboard.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include "chessfuncs.hpp"
#include "chesstypes.hpp"
#include "zobrist_bitboard_hash.hpp"


namespace C2_chess
{


Zobrist_bitboard_hash Bitboard::bb_hash_table;

inline void Bitboard::add_promotion_piece(piecetype p_type)
{
  switch (p_type)
  {
    case piecetype::Queen:
      (_col_to_move == col::white) ? _material_diff += 8.0 : _material_diff -= 8.0;
      break;
    case piecetype::Rook:
      (_col_to_move == col::white) ? _material_diff += 4.0 : _material_diff -= 4.0;
      break;
    case piecetype::Knight:
      (_col_to_move == col::white) ? _material_diff += 2.0 : _material_diff -= 2.0;
      break;
    case piecetype::Bishop:
      (_col_to_move == col::white) ? _material_diff += 2.0 : _material_diff -= 2.0;
      break;
    default:
      ;
  }
}

inline void Bitboard::touch_piece(uint64_t square)
{
  if (_col_to_move == col::white)
  {
    if (_W_Queens & square)
      _W_Queens ^= square;
    else if (_W_Pawns & square)
      _W_Pawns ^= square;
    else if (_W_King & square)
      _W_King ^= square;
    else if (_W_Rooks & square)
      _W_Rooks ^= square;
    else if (_W_Bishops & square)
      _W_Bishops ^= square;
    else if (_W_Knights & square)
      _W_Knights ^= square;
  }
  else
  {
    if (_B_Queens & square)
      _B_Queens ^= square;
    else if (_B_Pawns & square)
      _B_Pawns ^= square;
    else if (_B_King & square)
      _B_King ^= square;
    else if (_B_Rooks & square)
      _B_Rooks ^= square;
    else if (_B_Bishops & square)
      _B_Bishops ^= square;
    else if (_B_Knights & square)
      _B_Knights ^= square;
  }
}

inline piecetype Bitboard::get_piece_type(uint64_t square)
{
  if ((_W_Pawns & square) || (_B_Pawns & square))
  {
    return piecetype::Pawn;
  }
  else if ((_W_Queens & square) || (_B_Queens & square))
  {
    return piecetype::Queen;
  }
  else if ((_W_Rooks & square) || (_B_Rooks & square))
  {
    return piecetype::Rook;
  }
  else if ((_W_Bishops & square) || (_B_Bishops & square))
  {
    return piecetype::Bishop;
  }
  else if ((_W_Knights & square) || (_B_Knights & square))
  {
    return piecetype::Knight;
  }
  else if ((_W_King & square) || (_B_King & square))
  {
    return piecetype::King;
  }
  return piecetype::Undefined;
}

inline void Bitboard::remove_other_piece(uint64_t square)
{
  piecetype pt = piecetype::Undefined;
  if (_col_to_move == col::white)
  {
    if (_B_Queens & square)
    {
      _B_Queens ^= square, _material_diff += 9, pt = piecetype::Queen;
    }
    else if (_B_Pawns & square)
    {
      _B_Pawns ^= square, _material_diff += 1, pt = piecetype::Pawn;
    }
    else if (_B_Rooks & square)
    {
      _B_Rooks ^= square, _material_diff += 5, pt = piecetype::Rook;
      // update castling rights if needed
      if (square & h8_square)
        remove_castling_right(castling_right_BK);
      else if (square & a8_square)
        remove_castling_right(castling_right_BQ);
    }
    else if (_B_Bishops & square)
      _B_Bishops ^= square, _material_diff += 3, pt = piecetype::Bishop;
    else if (_B_Knights & square)
      _B_Knights ^= square, _material_diff += 3, pt = piecetype::Knight;
    update_hash_tag(square, col::black, pt);
  }
  else
  {
    if (_W_Queens & square)
      _W_Queens ^= square, _material_diff -= 9, pt = piecetype::Queen;
    else if (_W_Pawns & square)
      _W_Pawns ^= square, _material_diff -= 1, pt = piecetype::Pawn;
    else if (_W_Rooks & square)
    {
      _W_Rooks ^= square, _material_diff -= 5, pt = piecetype::Rook;
      // update castling rights if needed
      if (square & h1_square)
        remove_castling_right(castling_right_WK);
      else if (square & a1_square)
        remove_castling_right(castling_right_WQ);
    }
    else if (_W_Bishops & square)
      _W_Bishops ^= square, _material_diff -= 3, pt = piecetype::Bishop;
    else if (_W_Knights & square)
      _W_Knights ^= square, _material_diff -= 3, pt = piecetype::Knight;
    update_hash_tag(square, col::white, pt);
  }
}

inline void Bitboard::place_piece(piecetype p_type, uint64_t square)
{
  switch (p_type)
  {
    case piecetype::Pawn:
      (_col_to_move == col::white) ? _W_Pawns |= square : _B_Pawns |= square;
      break;
    case piecetype::Queen:
      (_col_to_move == col::white) ? _W_Queens |= square : _B_Queens |= square;
      break;
    case piecetype::King:
      (_col_to_move == col::white) ? _W_King |= square : _B_King |= square;
      break;
    case piecetype::Rook:
      (_col_to_move == col::white) ? _W_Rooks |= square : _B_Rooks |= square;
      break;
    case piecetype::Knight:
      (_col_to_move == col::white) ? _W_Knights |= square : _B_Knights |= square;
      break;
    case piecetype::Bishop:
      (_col_to_move == col::white) ? _W_Bishops |= square : _B_Bishops |= square;
      break;
    default:
      ;
  }
}

// Move the piece and update the hash_tag.
inline void Bitboard::move_piece(uint64_t from_square,
                                 uint64_t to_square,
                                 piecetype p_type)
{
  uint64_t* p = nullptr;
  switch (p_type)
  {
    case piecetype::Pawn:
      p = (_col_to_move == col::white) ? &_W_Pawns : &_B_Pawns;
      break;
    case piecetype::Queen:
      p = (_col_to_move == col::white) ? &_W_Queens : &_B_Queens;
      break;
    case piecetype::King:
      p = (_col_to_move == col::white) ? &_W_King : &_B_King;
      break;
    case piecetype::Rook:
      p = (_col_to_move == col::white) ? &_W_Rooks : &_B_Rooks;
      break;
    case piecetype::Knight:
      p = (_col_to_move == col::white) ? &_W_Knights : &_B_Knights;
      break;
    case piecetype::Bishop:
      p = (_col_to_move == col::white) ? &_W_Bishops : &_B_Bishops;
      break;
    default:
      ;
  }
  (*p) ^= (from_square | to_square);
  update_hash_tag(from_square, to_square, _col_to_move, p_type);
}

// Preconditions: Remove One castling_right at the time
inline void Bitboard::remove_castling_right(uint8_t cr)
{
  if (_castling_rights & cr)
  {
    _castling_rights ^= cr;
    _hash_tag ^= bb_hash_table._castling_rights[cr];
  }
}

inline void Bitboard::clear_ep_square()
{
  _hash_tag ^= bb_hash_table._en_passant_file[to_file(_ep_square)];
  _ep_square = zero;
}

inline void Bitboard::set_ep_square(uint64_t ep_square)
{
  _ep_square = ep_square;
  _hash_tag ^= bb_hash_table._en_passant_file[to_file(_ep_square)];
}

// Set a "unique" hash tag for the position after
// adding or removing one piece from a square.
inline void Bitboard::update_hash_tag(uint64_t square, col p_color, piecetype p_type)
{
  _hash_tag ^= bb_hash_table._random_table[bit_idx(square)][index(p_color)][index(p_type)];
}

// Set a "unique" hash tag for the position after
// removing one piece from a square and putting
// the same piece on an empty square.
inline void Bitboard::update_hash_tag(uint64_t square1, uint64_t square2, col p_color, piecetype p_type)
{
  _hash_tag ^= (bb_hash_table._random_table[bit_idx(square1)][index(p_color)][index(p_type)] ^
                bb_hash_table._random_table[bit_idx(square2)][index(p_color)][index(p_type)]);
}

void Bitboard::update_col_to_move()
{
  _col_to_move = other_color(_col_to_move);
  _hash_tag ^= bb_hash_table._black_to_move;
}

inline void Bitboard::update_state_after_king_move(const BitMove& m)
{
  uint64_t from_square = m._from_square, to_square = m._to_square;
  int8_t ri = m.to_r_index(), fi = m.to_f_index();

  if (_col_to_move == col::white)
  {
    // Could be slow.
    _W_King_file = file[fi], _W_King_file_index = fi;
    _W_King_rank = rank[ri], _W_King_rank_index = ri;
    _W_King_diagonal = diagonal[8 - ri + fi];
    _W_King_anti_diagonal = anti_diagonal[fi + ri - 1];
  }
  else
  {
    _B_King_file = file[fi], _B_King_file_index = fi;
    _B_King_rank = rank[ri], _B_King_rank_index = ri;
    _B_King_diagonal = diagonal[8 - ri + fi];
    _B_King_anti_diagonal = anti_diagonal[fi + ri - 1];
  }
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
  if (m._properties == move_props_castling)
  {
    if (static_cast<long int>(from_square - to_square) > 0)
    {
      // Castling.
      if (_col_to_move == col::white)
      {
        _W_Rooks ^= (h1_square | f1_square);
        update_hash_tag(h1_square, f1_square, _col_to_move, piecetype::Rook);
        _W_King_file >>= 2, _W_King_file_index += 2;
      }
      else
      {
        _B_Rooks ^= (h8_square | f8_square);
        update_hash_tag(h8_square, f8_square, _col_to_move, piecetype::Rook);
        _B_King_file <<= 2, _B_King_file_index -= 2;
      }
    }
    else
    {
      // Castling queen side.
      if (_col_to_move == col::white)
      {
        _W_Rooks ^= (a1_square | d1_square);
        update_hash_tag(a1_square, d1_square, _col_to_move, piecetype::Rook);
        _W_King_file <<= 3, _W_King_file_index -= 3;
      }
      else
      {
        _B_Rooks ^= (a8_square | d8_square);
        update_hash_tag(a8_square, d8_square, _col_to_move, piecetype::Rook);
        _B_King_file >>= 3, _B_King_file_index += 3;
      }
    }
  }
}

// Move a piece
// Preconditions:
//   i < movelist.size()
//   the move must be valid
void Bitboard::make_move(int i)
{
  BitMove m = _movelist[i];
  uint64_t to_square = m._to_square;
  uint64_t from_square = m._from_square;

// Clear _ep_square
  uint64_t tmp_ep_square = _ep_square;
  if (_ep_square)
    clear_ep_square();

// Remove possible piece of other color on to_square and
// Then make the move (updates hashtag)
  if (to_square & _s.other_pieces)
    remove_other_piece(to_square);
  move_piece(from_square, to_square, m._piece_type);

// OK we have moved the piece, now we must
// look at some special cases.
  if (m._piece_type == piecetype::King)
  {
    update_state_after_king_move(m);
  }
  else if (m._piece_type == piecetype::Rook)
  {
    // Clear castling rights for one side if applicable.
    if (to_square & _W_Rooks)
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
  else if (m._piece_type == piecetype::Pawn)
  {
    if (m._properties == move_props_en_passant)
    {
      // Remove the pawn taken e.p.
      if (_col_to_move == col::white)
      {
        _B_Pawns ^= tmp_ep_square << 8, _material_diff += 1.0;
        update_hash_tag(tmp_ep_square << 8, col::black, piecetype::Pawn);
      }
      else
      {
        _W_Pawns ^= tmp_ep_square >> 8, _material_diff -= 1.0;
        update_hash_tag(tmp_ep_square >> 8, col::black, piecetype::Pawn);
      }
      update_hash_tag(to_square, other_color(_col_to_move), piecetype::Pawn);
    }
    else if (_col_to_move == col::white)
    {
      // Set _ep_square if it's a two-square-move?
      if ((from_square & row_2) && (to_square & row_4))
      {
        // Check if there is a pawn of other color alongside to_square.
        if (((to_square & not_a_file) && ((to_square << 1) & _s.other_Pawns)) ||
            ((to_square & not_h_file) && ((to_square >> 1) & _s.other_Pawns)))
          set_ep_square(to_square << 8);
      }
    }
    else
    {
      if ((from_square & row_7) && (to_square & row_5))
      {
        if (((to_square & not_a_file) && ((to_square << 1) & _s.other_Pawns)) ||
            ((to_square & not_h_file) && ((to_square >> 1) & _s.other_Pawns)))
          set_ep_square(to_square >> 8);
      }
    }
  }
  else if (m._promotion_piece_type != piecetype::Undefined)
  {
    // Remove the pawn from promotion square
    // subtract 1 from the normal piece-values
    // because the pawn disappears from the board.
    (_col_to_move == col::white) ? _W_Pawns ^= to_square : _B_Pawns ^= to_square;
    update_hash_tag(to_square, other_color(_col_to_move), piecetype::Pawn);
    switch (m._promotion_piece_type)
    {
      case piecetype::Queen:
        (_col_to_move == col::white) ? _material_diff += 8.0, _W_Queens |= to_square :
                                       _material_diff -= 8.0, _B_Queens |= to_square;
        update_hash_tag(to_square, _col_to_move, piecetype::Queen);
        break;
      case piecetype::Rook:
        (_col_to_move == col::white) ? _material_diff += 4.0, _W_Rooks |= to_square :
                                       _material_diff -= 4.0, _B_Rooks |= to_square;
        update_hash_tag(to_square, _col_to_move, piecetype::Rook);
        break;
      case piecetype::Knight:
        (_col_to_move == col::white) ? _material_diff += 2.0, _W_Knights |= to_square :
                                       _material_diff -= 2.0, _B_Knights |= to_square;
        update_hash_tag(to_square, _col_to_move, piecetype::Knight);
        break;
      case piecetype::Bishop:
        (_col_to_move == col::white) ? _material_diff += 2.0, _W_Bishops |= to_square :
                                       _material_diff -= 2.0, _B_Bishops |= to_square;
        update_hash_tag(to_square, _col_to_move, piecetype::Bishop);
        break;
      default:
        ;
    }
  }
  update_col_to_move();
  init_piece_state();
  find_all_legal_moves();
}

} // namespace C2_chess



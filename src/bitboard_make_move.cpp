/*
 * Bitboard_make_move.cpp
 *
 *  Created on: Nov 3, 2021
 *      Author: torsten
 */

#include "bitboard.hpp"
#include "chesstypes.hpp"
#include "zobrist_bitboard_hash.hpp"

namespace C2_chess
{

Zobrist_bitboard_hash Bitboard::transposition_table;

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

piecetype Bitboard::get_piece_type(uint64_t square) const
{
  if ((_white_pieces.Pawns & square) || (_black_pieces.Pawns & square))
  {
    return piecetype::Pawn;
  }
  else if ((_white_pieces.Queens & square) || (_black_pieces.Queens & square))
  {
    return piecetype::Queen;
  }
  else if ((_white_pieces.Rooks & square) || (_black_pieces.Rooks & square))
  {
    return piecetype::Rook;
  }
  else if ((_white_pieces.Bishops & square) || (_black_pieces.Bishops & square))
  {
    return piecetype::Bishop;
  }
  else if ((_white_pieces.Knights & square) || (_black_pieces.Knights & square))
  {
    return piecetype::Knight;
  }
  else if ((_white_pieces.King & square) || (_black_pieces.King & square))
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
    if (_black_pieces.Queens & square)
    {
      _black_pieces.Queens ^= square, _material_diff += 9, pt = piecetype::Queen;
    }
    else if (_black_pieces.Pawns & square)
    {
      _black_pieces.Pawns ^= square, _material_diff += 1, pt = piecetype::Pawn;
    }
    else if (_black_pieces.Rooks & square)
    {
      _black_pieces.Rooks ^= square, _material_diff += 5, pt = piecetype::Rook;
      // update castling rights if needed
      if (square & h8_square)
        remove_castling_right(castling_right_BK);
      else if (square & a8_square)
        remove_castling_right(castling_right_BQ);
    }
    else if (_black_pieces.Bishops & square)
      _black_pieces.Bishops ^= square, _material_diff += 3, pt = piecetype::Bishop;
    else if (_black_pieces.Knights & square)
      _black_pieces.Knights ^= square, _material_diff += 3, pt = piecetype::Knight;
    update_hash_tag(square, col::black, pt);
  }
  else
  {
    if (_white_pieces.Queens & square)
      _white_pieces.Queens ^= square, _material_diff -= 9, pt = piecetype::Queen;
    else if (_white_pieces.Pawns & square)
    {
      _white_pieces.Pawns ^= square, _material_diff -= 1, pt = piecetype::Pawn;
    }
    else if (_white_pieces.Rooks & square)
    {
      _white_pieces.Rooks ^= square, _material_diff -= 5, pt = piecetype::Rook;
      // update castling rights if needed
      if (square & h1_square)
        remove_castling_right(castling_right_WK);
      else if (square & a1_square)
        remove_castling_right(castling_right_WQ);
    }
    else if (_white_pieces.Bishops & square)
      _white_pieces.Bishops ^= square, _material_diff -= 3, pt = piecetype::Bishop;
    else if (_white_pieces.Knights & square)
      _white_pieces.Knights ^= square, _material_diff -= 3, pt = piecetype::Knight;
    update_hash_tag(square, col::white, pt);
  }
}

inline void Bitboard::place_piece(piecetype p_type, uint64_t square)
{
  switch (p_type)
  {
    case piecetype::Pawn:
      (_col_to_move == col::white) ? _white_pieces.Pawns |= square : _black_pieces.Pawns |= square;
      break;
    case piecetype::Queen:
      (_col_to_move == col::white) ? _white_pieces.Queens |= square : _black_pieces.Queens |= square;
      break;
    case piecetype::King:
      (_col_to_move == col::white) ? _white_pieces.King |= square : _black_pieces.King |= square;
      break;
    case piecetype::Rook:
      (_col_to_move == col::white) ? _white_pieces.Rooks |= square : _black_pieces.Rooks |= square;
      break;
    case piecetype::Knight:
      (_col_to_move == col::white) ? _white_pieces.Knights |= square : _black_pieces.Knights |= square;
      break;
    case piecetype::Bishop:
      (_col_to_move == col::white) ? _white_pieces.Bishops |= square : _black_pieces.Bishops |= square;
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
      p = (_col_to_move == col::white) ? &_white_pieces.Pawns : &_black_pieces.Pawns;
      break;
    case piecetype::Queen:
      p = (_col_to_move == col::white) ? &_white_pieces.Queens : &_black_pieces.Queens;
      break;
    case piecetype::King:
      p = (_col_to_move == col::white) ? &_white_pieces.King : &_black_pieces.King;
      break;
    case piecetype::Rook:
      p = (_col_to_move == col::white) ? &_white_pieces.Rooks : &_black_pieces.Rooks;
      break;
    case piecetype::Knight:
      p = (_col_to_move == col::white) ? &_white_pieces.Knights : &_black_pieces.Knights;
      break;
    case piecetype::Bishop:
      p = (_col_to_move == col::white) ? &_white_pieces.Bishops : &_black_pieces.Bishops;
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
void Bitboard::update_hash_tag(uint64_t square, col p_color, piecetype p_type)
{
  _hash_tag ^= transposition_table._random_table[bit_idx(square)][index(p_color)][index(p_type)];
}

// Set a "unique" hash tag for the position after
// removing one piece from a square and putting
// the same piece on an empty square.
inline void Bitboard::update_hash_tag(uint64_t square1, uint64_t square2, col p_color, piecetype p_type)
{
  _hash_tag ^= (transposition_table._random_table[bit_idx(square1)][index(p_color)][index(p_type)] ^
                transposition_table._random_table[bit_idx(square2)][index(p_color)][index(p_type)]);
}

void Bitboard::update_col_to_move()
{
  _col_to_move = other_color(_col_to_move);
  if (_col_to_move == col::white)
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

inline void Bitboard::update_state_after_king_move(const BitMove& m)
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
      if (_col_to_move == col::white)
      {
        _white_pieces.Rooks ^= (h1_square | f1_square);
        update_hash_tag(h1_square, f1_square, _col_to_move, piecetype::Rook);
        _has_castled[index(col::white)] = true;
      }
      else
      {
        _black_pieces.Rooks ^= (h8_square | f8_square);
        update_hash_tag(h8_square, f8_square, _col_to_move, piecetype::Rook);
        _has_castled[index(col::black)] = true;
     }
    }
    else
    {
      // Castling queen side.
      if (_col_to_move == col::white)
      {
        _white_pieces.Rooks ^= (a1_square | d1_square);
        update_hash_tag(a1_square, d1_square, _col_to_move, piecetype::Rook);
        _has_castled[index(col::white)] = true;
      }
      else
      {
        _black_pieces.Rooks ^= (a8_square | d8_square);
        update_hash_tag(a8_square, d8_square, _col_to_move, piecetype::Rook);
        _has_castled[index(col::black)] = true;
      }
    }
  }
}

void Bitboard::make_move(uint8_t i, uint8_t& move_no)
{
  assert(i < _movelist.size());
  make_move(_movelist[i], move_no);
}

// The move must be valid, but doesn't have to be in _movelist.
// _movelist may be empty.
void Bitboard::make_move(const BitMove& m, uint8_t& move_no)
{
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
  if (m.piece_type() == piecetype::King)
  {
    update_state_after_king_move(m);
  }
  else if (m.piece_type() == piecetype::Rook)
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
  else if (m.piece_type() == piecetype::Pawn)
  {
    if (m.properties() & move_props_promotion)
    {
      // Remove the pawn from promotion square
      // subtract 1 from the normal piece-values
      // because the pawn disappears from the board.
      (_col_to_move == col::white) ? _white_pieces.Pawns ^= to_square : _black_pieces.Pawns ^= to_square;
      update_hash_tag(to_square, other_color(_col_to_move), piecetype::Pawn);
      switch (m.promotion_piece_type())
      {
        case piecetype::Queen:
          (_col_to_move == col::white) ? _material_diff += 8.0, _white_pieces.Queens |= to_square :
                                         _material_diff -= 8.0, _black_pieces.Queens |= to_square;
          update_hash_tag(to_square, _col_to_move, piecetype::Queen);
          break;
        case piecetype::Rook:
          (_col_to_move == col::white) ? _material_diff += 4.0, _white_pieces.Rooks |= to_square :
                                         _material_diff -= 4.0, _black_pieces.Rooks |= to_square;
          update_hash_tag(to_square, _col_to_move, piecetype::Rook);
          break;
        case piecetype::Knight:
          (_col_to_move == col::white) ? _material_diff += 2.0, _white_pieces.Knights |= to_square :
                                         _material_diff -= 2.0, _black_pieces.Knights |= to_square;
          update_hash_tag(to_square, _col_to_move, piecetype::Knight);
          break;
        case piecetype::Bishop:
          (_col_to_move == col::white) ? _material_diff += 2.0, _white_pieces.Bishops |= to_square :
                                         _material_diff -= 2.0, _black_pieces.Bishops |= to_square;
          update_hash_tag(to_square, _col_to_move, piecetype::Bishop);
          break;
        default:
          ;
      }
    }
    else if (m.properties() & move_props_en_passant)
    {
      // Remove the pawn taken e.p.
      if (_col_to_move == col::white)
      {
        _black_pieces.Pawns ^= tmp_ep_square << 8, _material_diff += 1.0;
        update_hash_tag(tmp_ep_square << 8, col::black, piecetype::Pawn);
      }
      else
      {
        _white_pieces.Pawns ^= tmp_ep_square >> 8, _material_diff -= 1.0;
        update_hash_tag(tmp_ep_square >> 8, col::black, piecetype::Pawn);
      }
      update_hash_tag(to_square, other_color(_col_to_move), piecetype::Pawn);
    }
    else if ((_col_to_move == col::white) && (from_square & row_2) && (to_square & row_4))
    {
      // Check if there is a pawn of other color alongside to_square.
      if (((to_square & not_a_file) && ((to_square << 1) & _other->Pawns)) ||
          ((to_square & not_h_file) && ((to_square >> 1) & _other->Pawns)))
        set_ep_square(to_square << 8);
    }
    else if ((_col_to_move == col::black) && (from_square & row_7) && (to_square & row_5))
    {
      if (((to_square & not_a_file) && ((to_square << 1) & _other->Pawns)) ||
          ((to_square & not_h_file) && ((to_square >> 1) & _other->Pawns)))
        set_ep_square(to_square >> 8);
    }
  }
  // Set up the board for other player:
  _last_move = m;
  update_col_to_move();
  if (_col_to_move == col::white)
    move_no++;
  if (square_is_threatened(_own->King, false))
    _last_move.add_property(move_props_check);
  find_all_legal_moves();
}

}
// namespace C2_chess


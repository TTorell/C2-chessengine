/*
 * bitboard_movegen.cpp
 *
 *  Created on: 11 nov. 2021
 *      Author: torsten
 */
#include "bitboard.hpp"
#include "chesstypes.hpp"
#include "current_time.hpp"
#include "transposition_table.hpp"

namespace C2_chess
{

void Bitboard::find_long_castling()
{
  uint64_t King_initial_square = (_col_to_move == col::white) ? e1_square : e8_square;
  if (_castling_rights & ((_col_to_move == col::white) ? castling_right_WQ : castling_right_BQ))
  {
    uint64_t castling_empty_squares = (castling_empty_squares_Q & ((_col_to_move == col::white) ? lower_board_half : upper_board_half));
    // The squares between king and rook must be empty.
    if (castling_empty_squares & _all_pieces)
      return;
    // The squares between king and rook on d- and c-file must not be threatened,
    // but the square on the b-file may be.
    castling_empty_squares &= ~b_file;
    while (castling_empty_squares)
    {
      if (square_is_threatened(popright_square(castling_empty_squares), false))
        return;
    }
    _movelist.push_back(BitMove(piecetype::King, move_props_castling, King_initial_square, King_initial_square << 2));
  }
}

void Bitboard::find_short_castling()
{
  uint64_t King_initial_square = (_col_to_move == col::white) ? e1_square : e8_square;
  if (_castling_rights & ((_col_to_move == col::white) ? castling_right_WK : castling_right_BK))
  {
    uint64_t castling_empty_squares = (castling_empty_squares_K & ((_col_to_move == col::white) ? lower_board_half : upper_board_half));
    if (castling_empty_squares & _all_pieces)
      return;
    while (castling_empty_squares)
    {
      if (square_is_threatened(popright_square(castling_empty_squares), false))
        return;
    }
    _movelist.push_back(BitMove(piecetype::King, move_props_castling, King_initial_square, King_initial_square >> 2));
  }
}

inline uint64_t Bitboard::find_blockers(uint64_t sq, uint64_t mask, uint64_t all_pieces)
{
  uint64_t left_side, right_side;
  mask ^= sq;
  left_side = mask & all_pieces & ~(sq - 1);
  right_side = mask & all_pieces & (sq - 1);
  return leftmost_square(right_side) | rightmost_square(left_side);
}

inline uint64_t Bitboard::find_other_color_blockers(uint64_t sq, uint64_t mask)
{
  uint64_t left_side, right_side;
  mask ^= sq;
  left_side = mask & _all_pieces & ~(sq - 1);
  right_side = mask & _all_pieces & (sq - 1);
  return (leftmost_square(right_side) | rightmost_square(left_side)) & _other->pieces;
}

uint64_t Bitboard::find_legal_squares(uint64_t sq, uint64_t mask)
{
  mask ^= sq;
  uint64_t left_side = mask & _all_pieces & ~(sq - 1);
  uint64_t left_blocker = rightmost_square(left_side);
  uint64_t right_side = mask & _all_pieces & (sq - 1);
  uint64_t right_blocker = leftmost_square(right_side);
  uint64_t other_color_blockers = (left_blocker | right_blocker) & _other->pieces;
  if (right_blocker)
    return (((left_blocker - one) ^ ((right_blocker << 1) - one)) | other_color_blockers) & mask;
  else
    return ((left_blocker - one) | other_color_blockers) & mask;
}

void Bitboard::find_Queen_Rook_and_Bishop_moves(gentype gt)
{
  uint64_t from_square;
  uint64_t to_square;
  uint64_t legal_squares = zero;
  uint64_t pieces = (_own->Queens | _own->Rooks | _own->Bishops) & ~_pinned_pieces;
  while (pieces)
  {
    from_square = popright_square(pieces);
    piecetype p_type = (from_square & _own->Queens) ? piecetype::Queen :
                       (from_square & _own->Rooks) ? piecetype::Rook : piecetype::Bishop;
    if (p_type != piecetype::Bishop)
    {
      if (gt == gentype::all)
      {
        legal_squares |= find_legal_squares(from_square, file[file_idx(from_square)]);
        legal_squares |= find_legal_squares(from_square, rank[rank_idx(from_square)]);
      }
      else
      {
        legal_squares |= find_other_color_blockers(from_square, file[file_idx(from_square)]);
        legal_squares |= find_other_color_blockers(from_square, rank[rank_idx(from_square)]);
      }
    }
    if (p_type != piecetype::Rook)
    {
      if (gt == gentype::all)
      {
        legal_squares |= find_legal_squares(from_square, to_diagonal(from_square));
        legal_squares |= find_legal_squares(from_square, to_anti_diagonal(from_square));
      }
      else
      {
        legal_squares |= find_other_color_blockers(from_square, to_diagonal(from_square));
        legal_squares |= find_other_color_blockers(from_square, to_anti_diagonal(from_square));
      }
    }
    while (legal_squares)
    {
      to_square = popright_square(legal_squares);
      if (to_square & _other->pieces)
        add_move(p_type, move_props_capture, from_square, to_square);
      else
        _movelist.push_back(BitMove(p_type, move_props_none, from_square, to_square));
    }
  }
}

void Bitboard::find_legal_moves_for_pinned_pieces(gentype gt)
{
  assert(_pinned_pieces & ~_own->Pawns); // Pinned Pawns has been taken care of.
  uint64_t from_square;
  uint64_t to_square;
  uint64_t legal_squares = zero;
  uint64_t King_file_idx = file_idx(_own->King);
  uint64_t King_rank_idx = rank_idx(_own->King);
  uint64_t pieces = (_own->Queens | _own->Rooks | _own->Bishops) & _pinned_pieces;
  while (pieces)
  {
    from_square = popright_square(pieces);
    piecetype p_type = (from_square & _own->Queens) ? piecetype::Queen :
                       (from_square & _own->Rooks) ? piecetype::Rook : piecetype::Bishop;

    // Here we know that "from_square" is somewhere between our King and a piece of other color.
    if (p_type != piecetype::Bishop)
    {
      if (gt == gentype::all)
      {
        if (from_square & file[King_file_idx])
          legal_squares |= find_legal_squares(from_square, file[King_file_idx]);
        else if (from_square & rank[King_rank_idx])
          legal_squares |= find_legal_squares(from_square, rank[King_rank_idx]);
      }
      else
      {
        if (from_square & file[King_file_idx])
          legal_squares |= find_other_color_blockers(from_square, file[King_file_idx]);
        else if (from_square & rank[King_rank_idx])
          legal_squares |= find_other_color_blockers(from_square, rank[King_rank_idx]);
      }
    }
    if (p_type != piecetype::Rook)
    {
      if (gt == gentype::all)
      {
        if (from_square & to_diagonal(King_file_idx, King_rank_idx))
          legal_squares |= find_legal_squares(from_square, to_diagonal(King_file_idx, King_rank_idx));
        else if (from_square & to_anti_diagonal(King_file_idx, King_rank_idx))
          legal_squares |= find_legal_squares(from_square, to_anti_diagonal(King_file_idx, King_rank_idx));
      }
      else
      {
        if (from_square & to_diagonal(King_file_idx, King_rank_idx))
          legal_squares |= find_other_color_blockers(from_square, to_diagonal(King_file_idx, King_rank_idx));
        else if (from_square & to_anti_diagonal(King_file_idx, King_rank_idx))
          legal_squares |= find_other_color_blockers(from_square, to_anti_diagonal(King_file_idx, King_rank_idx));
      }
    }
    while (legal_squares)
    {
      to_square = popright_square(legal_squares);
      if (to_square & _other->pieces)
        add_move(p_type, move_props_capture, from_square, to_square);
      else
        _movelist.push_back(BitMove(p_type, move_props_none, from_square, to_square));
    }
  }
}

void Bitboard::find_Knight_moves(gentype gt)
{
  uint64_t from_square;
  uint64_t to_square;
  uint64_t to_squares;
  if (_own->Knights)
  {
    uint64_t knights = _own->Knights & ~_pinned_pieces;
    while (knights)
    {
      from_square = popright_square(knights);
      if (gt == gentype::all)
      {
        to_squares = adjust_pattern(knight_pattern, from_square) & ~_own->pieces;
        while (to_squares)
        {
          to_square = popright_square(to_squares);
          if (to_square & _other->pieces)
            add_move(piecetype::Knight, move_props_capture, from_square, to_square);
          else
            _movelist.push_back(BitMove(piecetype::Knight, move_props_none, from_square, to_square));
        }
      }
      else
      {
        to_squares = adjust_pattern(knight_pattern, from_square) & _other->pieces;
        while (to_squares)
        {
          to_square = popright_square(to_squares);
          add_move(piecetype::Knight, move_props_capture, from_square, to_square);
        }

      }
    }
  }
}

void Bitboard::find_Pawn_moves(gentype gt)
{
  uint64_t to_square;
  uint64_t moved_pawns;
  uint64_t King_file_idx = file_idx(_own->King);
  uint64_t King_rank_idx = rank_idx(_own->King);
  uint64_t King_file = file[King_file_idx];
  uint64_t King_diagonal = to_diagonal(King_file_idx, King_rank_idx);
  uint64_t King_anti_diagonal = to_anti_diagonal(King_file_idx, King_rank_idx);

  // A pinned pawn on the King-file can possibly move.
  uint64_t pawns = _own->Pawns & ~(_pinned_pieces & ~King_file);
  // Shift chosen pawns one square ahead. Check if the new square is empty:
  pawns = ((_col_to_move == col::white) ? pawns >> 8 : pawns << 8) & ~_all_pieces;
  if (gt == gentype::all)
  {
    // save moved pieces
    moved_pawns = pawns;
    while (pawns)
    {
      to_square = popright_square(pawns);
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 8 : to_square >> 8, to_square);
    }
    // Check if any of the moved pawns can take another step forward
    if (moved_pawns)
    {
      moved_pawns = (~_all_pieces) & ((_col_to_move == col::white) ? ((moved_pawns >> 8) & rank[4]) :
                                                                     ((moved_pawns << 8) & rank[5]));
      while (moved_pawns)
      {
        to_square = popright_square(moved_pawns);
        _movelist.push_back(BitMove(piecetype::Pawn,
                                    move_props_none,
                                    (_col_to_move == col::white) ? to_square << 16 : to_square >> 16,
                                    to_square));
      }
    }
  }
  if (gt == gentype::captures)
  {
    // Include normal promotions in the "capture"-category
    pawns = pawns & ((_col_to_move == col::white) ? rank[8] : rank[1]);
    while (pawns)
    {
      to_square = popright_square(pawns);
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 8 : to_square >> 8, to_square);
    }
  }

  // Check for pawn captures and ep.
  // Pinned pawns along King-diagonal may still be able to capture the pinning piece
  pawns = _own->Pawns & ~(_pinned_pieces & ~King_diagonal);
  moved_pawns = (_col_to_move == col::white) ? (pawns >> 9) & not_a_file : (pawns << 9) & not_h_file;
  while (moved_pawns)
  {
    to_square = popright_square(moved_pawns);
    if (to_square & _other->pieces)
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 9 :
                                                                   to_square >> 9,
                                    to_square);
    else if (to_square == _ep_square)
      try_adding_ep_pawn_move((_col_to_move == col::white) ? to_square << 9 : to_square >> 9);
  }
  pawns = _own->Pawns & ~(_pinned_pieces & ~King_anti_diagonal);
  moved_pawns = (_col_to_move == col::white) ? (pawns >> 7) & not_h_file : (pawns << 7) & not_a_file;
  while (moved_pawns)
  {
    to_square = popright_square(moved_pawns);
    if (to_square & _other->pieces)
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 7 :
                                                                   to_square >> 7,
                                    to_square);
    else if (to_square == _ep_square)
      try_adding_ep_pawn_move((_col_to_move == col::white) ? to_square << 7 : to_square >> 7);
  }
}

void Bitboard::find_normal_legal_moves(gentype gt)
{
  find_Pawn_moves(gt);
  find_Knight_moves(gt);
  if (_pinned_pieces & ~_own->Pawns)
    find_legal_moves_for_pinned_pieces(gt);
  find_Queen_Rook_and_Bishop_moves(gt);
  if (gt == gentype::all)
  {
    if ((_col_to_move == col::white && _own->King == e1_square) ||
        (_col_to_move == col::black && _own->King == e8_square))
    {
      find_short_castling();
      find_long_castling();
    }
  }
  // Other King-moves have already been taken care of.

}

void Bitboard::find_Knight_moves_to_square(const uint64_t to_square)
{
  assert(to_square & ~_own->pieces);
  if (_own->Knights)
  {
    uint64_t knights = adjust_pattern(knight_pattern, to_square) & _own->Knights;
    while (knights)
    {
      uint64_t knight = popright_square(knights);
      if (knight & ~_pinned_pieces)
      {
        if (to_square & _other->pieces)
          add_move(piecetype::Knight, move_props_capture, knight, to_square);
        else
          _movelist.push_back(BitMove(piecetype::Knight, move_props_none, knight, to_square));
      }
    }
  }
}

bool Bitboard::check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square)
{
  uint64_t possible_pinners;
  uint64_t possible_pinner;
  uint64_t between_squares;
  uint64_t other_Queens_or_Rooks = _other->Queens | _other->Rooks;
  uint64_t other_Queens_or_Bishops = _other->Queens | _other->Bishops;
// Check file and rank
  if (other_Queens_or_Rooks)
    if (_other->Queens | _other->Rooks)
    {
      uint64_t King_ortogonal_squares = ortogonal_squares(_own->King);
      if (King_ortogonal_squares & other_pawn_square)
      {
        possible_pinners = King_ortogonal_squares & other_Queens_or_Rooks;
        while (possible_pinners)
        {
          possible_pinner = popright_square(possible_pinners);
          between_squares = between(_own->King, possible_pinner, King_ortogonal_squares);
          if (between_squares & other_pawn_square)
          {
            if (std::has_single_bit(between_squares & _all_pieces))
              return true;
            // own_pawn_square can be a blocker on a rank together with other_pawn_square
            // which both would be gone from the rank after the ep-move.
            if (rank[rank_idx(_own->King)] & other_pawn_square)
            {
              if ((std::popcount(between_squares & _all_pieces) == 2) && (between_squares & own_pawn_square))
              {
                return true;
              }
            }
          }
        }
      }
    }
  if (_other->Queens | _other->Bishops)
  {
    uint64_t King_diagonal_squares = diagonal_squares(_own->King);
    if (King_diagonal_squares & other_pawn_square)
    {
      possible_pinners = King_diagonal_squares & other_Queens_or_Bishops;
      while (possible_pinners)
      {
        possible_pinner = popright_square(possible_pinners);
        between_squares = between(_own->King, possible_pinner, King_diagonal_squares, true);
        if (between_squares & other_pawn_square)
        {
          if (std::has_single_bit(between_squares & _all_pieces))
            return true;
        }
      }
    }
  }
  return false;
}

// Check if en passant is possible and if so, add the move.
// Facts:
// - We don't have to consider promotion. E.p. can never be
//   a promotion.
// Preconditions:
// - from_square should contain a pawn of color _col_to_move.
// - _ep_square may not be OL.
// - from square must be located such that ep_capture
//   to _ep_square is possible.
void Bitboard::try_adding_ep_pawn_move(uint64_t from_square)
{
  assert(std::has_single_bit(from_square) && _ep_square &&
         (abs(static_cast<int64_t>(bit_idx(_ep_square) - bit_idx(from_square))) == 7 ||
          abs(static_cast<int64_t>(bit_idx(_ep_square) - bit_idx(from_square))) == 9));

  if (from_square & ~_pinned_pieces)
  {
// Our pawn is not pinned.
// Here we must check that our King won't be in check when
// the taken pawn is removed. There is also a special case
// on rank 5 (or 4 for black) where the king can be in a check
// from the side when both the taken pawn and our own pawn
// disappears from the rank.
    uint64_t other_pawn_square = (_col_to_move == col::white) ? _ep_square << 8 : _ep_square >> 8;

// A pin (of the other pawn) on the king-file poses no problem,
// since our own pawn will block the file after the capture.
    if ((other_pawn_square & to_file(_own->King)) == zero)
    {
      if (check_if_other_pawn_is_pinned_ep(other_pawn_square, from_square))
        return;
    }
    add_move(piecetype::Pawn,
             move_props_capture | move_props_en_passant,
             from_square,
             _ep_square);
  }
  else
  {
// Our own pawn is pinned, but can still capture e.p. along the
// diagonal of the king.
    if (_ep_square & (to_diagonal(_own->King) | to_anti_diagonal(_own->King)))
    {
      // No need to check if other_pawn is pinned, it can't be.
      add_move(piecetype::Pawn,
               move_props_capture | move_props_en_passant,
               from_square,
               _ep_square);
    }
  }
}

// - The location of from_square and to_square should allow some kind of
//   pawn-move from from_square to to_square.

// Adding Pawn move considering promotion of the Pawn.
void Bitboard::add_pawn_move_check_promotion(uint64_t from_square,
                                             uint64_t to_square)
{
  assert(std::has_single_bit(from_square) && (from_square & _own->Pawns) &&
         std::has_single_bit(to_square));

  if ((_col_to_move == col::white) ? to_square & not_row_8 : to_square & not_row_1)
  {
    if (to_square & ~_all_pieces)
    {
      // To empty squares we only allow "straight" pawn-moves. (or e.p., already taken care of)
      if (to_square == (_col_to_move == col::white) ? from_square >> 8 : from_square << 8)
        _movelist.push_back(BitMove(piecetype::Pawn,
                                    move_props_none,
                                    from_square,
                                    to_square));
    }
    else if (to_square & _other->pieces)
    {
      add_move(piecetype::Pawn,
               move_props_capture,
               from_square,
               to_square);
    }
  }
  else
  {
    // Promotion
    uint8_t move_props = move_props_promotion;
    if (to_square & _other->pieces)
      move_props |= move_props_capture;

    add_move(piecetype::Pawn, move_props, from_square, to_square, piecetype::Bishop);
    add_move(piecetype::Pawn, move_props, from_square, to_square, piecetype::Knight);
    add_move(piecetype::Pawn, move_props, from_square, to_square, piecetype::Rook);
    add_move(piecetype::Pawn, move_props, from_square, to_square); // Queen
  }
}

// preconditions:
// - to_square should be empty
void Bitboard::find_pawn_moves_to_empty_square(uint64_t to_square, gentype gt)
{
  assert(std::has_single_bit(to_square) && (to_square & ~_all_pieces));
  uint64_t from_square;

  if (gt == gentype::all)
  {
    from_square = (_col_to_move == col::white) ? to_square << 8 : to_square >> 8; // one step
    // No pawn moves possible if from_sqare doesn't contain a pawn of
    // color _col_to_move, which isn't pinned.
    if ((from_square & _own->Pawns) && (from_square & ~_pinned_pieces))
    {
      add_pawn_move_check_promotion(from_square, to_square);
    }
    else if ((from_square & ~_all_pieces) && (to_square & ((_col_to_move == col::white) ? row_4 : row_5)))
    {
      // No blocking piece and correct row for a for two-squares-pawn-move
      (_col_to_move == col::white) ? from_square <<= 8 : from_square >>= 8; // two steps
      if ((from_square & _own->Pawns) && (from_square & ~_pinned_pieces))
      {
        _movelist.push_back(BitMove(piecetype::Pawn,
                                    move_props_none,
                                    from_square,
                                    to_square));
      }
    }
  }

// Normal pawn captures are not possible since to_square is empty.
// En passant, however, is possible.
  if (to_square & _ep_square)
  {
    if (_ep_square & not_h_file)
    {
      from_square = (_col_to_move == col::white) ? _ep_square << 7 : _ep_square >> 9;
      if (from_square & _own->Pawns)
        try_adding_ep_pawn_move(from_square);
    }
    if (_ep_square & not_a_file)
    {
      from_square = (_col_to_move == col::white) ? _ep_square << 9 : _ep_square >> 7;
      if (from_square & _own->Pawns)
        try_adding_ep_pawn_move(from_square);
    }
  }
}

// The following method doesn't consider possible King-moves to square
void Bitboard::find_moves_to_square(uint64_t to_square, gentype gt)
{
  assert((to_square & ~_own->pieces) && std::has_single_bit(to_square));
  uint64_t from_square;
  uint64_t move_candidates;
  uint64_t move_candidate;
  uint64_t between_squares;
  if (to_square & ~_all_pieces)
  {
    find_pawn_moves_to_empty_square(to_square, gt);
  }
  else if (to_square & _other->pieces)
  {
    // Pawn Captures (e.p. not possible)
    if (to_square & not_h_file)
    {
      from_square = (_col_to_move == col::white) ? to_square << 7 : to_square >> 9;
      if ((from_square & _own->Pawns) && (from_square & ~_pinned_pieces))
        add_pawn_move_check_promotion(from_square, to_square);
    }
    if (to_square & not_a_file)
    {
      from_square = (_col_to_move == col::white) ? to_square << 9 : to_square >> 7;
      if ((from_square & _own->Pawns) && (from_square & ~_pinned_pieces))
        add_pawn_move_check_promotion(from_square, to_square);
    }
  }

  if (gt == gentype::all || (to_square & _other->pieces))
  {
    find_Knight_moves_to_square(to_square);

    // Check file and rank
    uint64_t Queens_or_Rooks = _own->Queens | _own->Rooks;
    if (Queens_or_Rooks)
    {
      uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
      move_candidates = to_ortogonal_squares & Queens_or_Rooks;
      while (move_candidates)
      {
        move_candidate = popright_square(move_candidates);
        if (move_candidate & ~_pinned_pieces)
        {
          between_squares = between(to_square, move_candidate, to_ortogonal_squares);
          if ((between_squares & _all_pieces) == zero)
          {
            piecetype p_type = (move_candidate & _own->Queens) ? piecetype::Queen : piecetype::Rook;
            if (to_square & ~_all_pieces)
              _movelist.push_back(BitMove(p_type, move_props_none, move_candidate, to_square));
            else if (to_square & _other->pieces)
              add_move(p_type, move_props_capture, move_candidate, to_square);
          }
        }
      }
    }

    // Check diagonals
    uint64_t Queens_or_Bishops = _own->Queens | _own->Bishops;
    if (Queens_or_Bishops)
    {
      uint64_t to_diagonal_squares = diagonal_squares(to_square);
      move_candidates = to_diagonal_squares & Queens_or_Bishops;
      while (move_candidates)
      {
        move_candidate = popright_square(move_candidates);
        if (move_candidate & ~_pinned_pieces)
        {
          between_squares = between(to_square, move_candidate, to_diagonal_squares, true); // true = diagonal
          if ((between_squares & _all_pieces) == zero)
          {
            piecetype p_type = (move_candidate & _own->Queens) ? piecetype::Queen : piecetype::Bishop;
            if (to_square & ~_all_pieces)
              _movelist.push_back(BitMove(p_type, move_props_none, move_candidate, to_square));
            else if (to_square & _other->pieces)
              add_move(p_type, move_props_capture, move_candidate, to_square);
          }
        }
      }
    }
  }
}

void Bitboard::find_moves_after_check(gentype gt)
{
  assert(std::has_single_bit(_checkers));
  // All possible King-moves have already been found.
  uint64_t between_squares;
  uint64_t between_square;

  if (gt == gentype::all || _ep_square)
  {
    uint64_t King_ortogonal_squares = ortogonal_squares(_own->King);
    if (_checkers & King_ortogonal_squares)
    {
      // Can we put any piece between the King and the checker?
      between_squares = between(_own->King, _checkers, King_ortogonal_squares);
      while (between_squares)
      {
        between_square = popright_square(between_squares);
        find_moves_to_square(between_square, gt);
      }
    }
    else
    {
      uint64_t King_diagonal_squares = diagonal_squares(_own->King);
      if (_checkers & King_diagonal_squares)
      {
        // Can we put any piece between the King and the checker?
        between_squares = between(_own->King, _checkers, King_diagonal_squares, true);
        while (between_squares)
        {
          between_square = popright_square(between_squares);
          find_moves_to_square(between_square, gt);
        }
      }
    }
  }

   // Can we take the checking piece?
  find_moves_to_square(_checkers, gt);

  // Also check if the checker is a Pawn which can be taken e.p.
  if (_ep_square & ((_col_to_move == col::white) ? _checkers >> 8 : _checkers << 8))
  {
    if (_ep_square & not_a_file)
    {
      uint64_t from_square = (_col_to_move == col::white) ? _ep_square << 9 : _ep_square >> 7;
      if (from_square & _own->Pawns)
        try_adding_ep_pawn_move(from_square);
    }
    if (_ep_square & not_h_file)
    {
      uint64_t from_square = (_col_to_move == col::white) ? _ep_square << 7 : _ep_square >> 9;
      if (from_square & _own->Pawns)
        try_adding_ep_pawn_move(from_square);
    }
  }
}

// --------------------------------------------------------
// Detects check, double check and pinned_pieces.
//
// Returns the number of checks found.
//
// The squares of Pinned pieces are stored in the member
// variable _pinned_pieces and the square of the last
// found check is stored in _checking_piece_square.
//
// Knowing about this can speed up the process of
// figuring out all possible moves in a position.
// On a double check only King moves are possible.
// On a single check we must also check if we can capture
// the checking piece or put a piece in between, but still
// we can skip checking all other more normal moves.
//
// When it comes to double checks I've figured out the
// following logic restrictions which could be useful:
// - A double check must contain a discovered check.
// - The checking piece that is discovered can of course never
//   be a Knight or pawn.
// - A double check can never consist of two diagonal checks.
// - A double check can never consist of two checks along
//   files and/or ranks.
// - A double check can never consist of two checks from
//   pieces of the same type.
// - A double check can never consist of a Knight-check and
//   a Pawn-check.
// - A check from a Queen on adjacent square to the King can
//   never be part of a double check.
//
// But naturally, I haven't made use of all these
// restrictions.
// ------------------------------------------------------
void Bitboard::find_checkers_and_pinned_pieces()
{
  uint64_t possible_checkers;
  uint64_t possible_checker;
  uint64_t between_squares;
  uint8_t King_file_idx = file_idx(_own->King);
  uint64_t other_Queens_or_Rooks = _other->Queens | _other->Rooks;
  uint64_t other_Queens_or_Bishops = _other->Queens | _other->Bishops;

// Check Pawn-threats
  if (_other->Pawns)
  {
    if (King_file_idx != h)
    {
      possible_checker = (_col_to_move == col::white) ? _own->King >> 9 : _own->King << 7;
      if (possible_checker & _other->Pawns)
        _checkers |= possible_checker;
    }
    if (King_file_idx != a)
    {
      possible_checker = (_col_to_move == col::white) ? _own->King >> 7 : _own->King << 9;
      if (possible_checker & _other->Pawns)
        _checkers |= possible_checker;
    }
  }

// Check Knight threats
  if (_other->Knights)
    _checkers |= (adjust_pattern(knight_pattern, _own->King) & _other->Knights);

// Check threats on file and rank
  if (_other->Queens | _other->Rooks)
  {
    uint64_t King_ortogonal_squares = ortogonal_squares(_own->King);
    possible_checkers = King_ortogonal_squares & (other_Queens_or_Rooks);
    while (possible_checkers)
    {
      possible_checker = popright_square(possible_checkers);
      between_squares = between(_own->King, possible_checker, King_ortogonal_squares);
      if ((between_squares & _all_pieces) == zero)
        _checkers |= possible_checker;
      else if (std::has_single_bit(between_squares & _all_pieces))
      {
        _pinned_pieces |= (between_squares & _own->pieces);
        _pinners |= possible_checker;
      }
    }
  }

// Check diagonal threats
  if (_other->Queens | _other->Bishops)
  {
    uint64_t King_diagonal_squares = diagonal_squares(_own->King);
    possible_checkers = King_diagonal_squares & (other_Queens_or_Bishops);
    while (possible_checkers)
    {
      possible_checker = popright_square(possible_checkers);
      between_squares = between(_own->King, possible_checker, King_diagonal_squares, true); // true = diagonal
      if ((between_squares & _all_pieces) == zero)
        _checkers |= possible_checker;
      else if (std::has_single_bit(between_squares & _all_pieces))
      {
        _pinned_pieces |= (between_squares & _own->pieces);
        _pinners |= possible_checker;
      }
    }
  }
}

bool Bitboard::square_is_threatened(uint64_t to_square, bool King_is_asking) const
{
  uint64_t possible_attackers;
  uint64_t attacker;
  uint64_t tmp_all_pieces = _all_pieces;
  uint8_t f_idx = file_idx(to_square);

// Check Pawn-threats
  if (_other->Pawns)
  {
    if ((f_idx != h) && (_other->Pawns & ((_col_to_move == col::white) ? to_square >> 9 : to_square << 7)))
      return true;
    if ((f_idx != a) && (_other->Pawns & ((_col_to_move == col::white) ? to_square >> 7 : to_square << 9)))
      return true;
  }

// Check Knight-threats
  if (_other->Knights && ((adjust_pattern(knight_pattern, to_square) & _other->Knights)))
    return true;

// Check King (and adjacent Queen-threats)
  if (adjust_pattern(king_pattern, to_square) & (_other->King | _other->Queens))
    return true;

  if (King_is_asking)
  {
    // Treat King-square as empty to catch xray-attacks through the King-square
    tmp_all_pieces ^= _own->King;
  }

// Check threats on file and rank
  if (_other->Queens | _other->Rooks)
  {
    // Check threats on file and rank
    uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
    possible_attackers = to_ortogonal_squares & (_other->Queens | _other->Rooks);
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_ortogonal_squares) & tmp_all_pieces) == zero)
        return true;
    }
  }

// Check diagonal threats
  if (_other->Queens | _other->Bishops)
  {
    uint64_t to_diagonal_squares = diagonal_squares(to_square);
    possible_attackers = to_diagonal_squares & (_other->Queens | _other->Bishops);
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_diagonal_squares, true) & tmp_all_pieces) == zero)
        return true;
    }
  }
  return false;
}

// An attempt to speed up the square_is_threatened() method, but it
// takes about equal time. Sometimes faster, sometimes slower, depending
// on the position.
bool Bitboard::square_is_threatened2(uint64_t to_square, bool King_is_asking)
{
  uint64_t other_Queens_or_rooks, other_Queens_or_Bishops;
  uint64_t tmp_all_pieces = _all_pieces;
  uint8_t to_bit_idx = bit_idx(to_square);
  uint8_t f_idx = file_idx(to_bit_idx);
  uint8_t r_idx = rank_idx(to_bit_idx);

// Check Pawn-threats
  if (_other->Pawns)
  {
    if ((f_idx != h) && (_other->Pawns & ((_col_to_move == col::white) ? to_square >> 9 : to_square << 7)))
      return true;
    if ((f_idx != a) && (_other->Pawns & ((_col_to_move == col::white) ? to_square >> 7 : to_square << 9)))
      return true;
  }

// Check Knight-threats
  if (_other->Knights && ((adjust_pattern(knight_pattern, to_square) & _other->Knights)))
    return true;

// Check King (and adjacent Queen-threats)
  if (adjust_pattern(king_pattern, to_square) & (_other->King | _other->Queens))
    return true;

  if (King_is_asking)
  {
    // Treat King-square as empty to catch xray-attacks through the King-square
    tmp_all_pieces ^= _own->King;
  }

// Check threats on file and rank
  other_Queens_or_rooks = _other->Queens | _other->Rooks;
  if (other_Queens_or_rooks)
  {
    if (find_blockers(to_square, file[f_idx], tmp_all_pieces) & other_Queens_or_rooks)
      return true;
    if (find_blockers(to_square, rank[r_idx], tmp_all_pieces) & other_Queens_or_rooks)
      return true;
  }

// Check diagonal threats
  other_Queens_or_Bishops = _other->Queens | _other->Bishops;
  if (other_Queens_or_Bishops)
  {
    if (find_blockers(to_square, to_diagonal(f_idx, r_idx), tmp_all_pieces) & other_Queens_or_Bishops)
      return true;
    if (find_blockers(to_square, to_anti_diagonal(f_idx, r_idx), tmp_all_pieces) & other_Queens_or_Bishops)
      return true;
  }

  return false;
}

// Finds normal King-moves not including castling.
inline void Bitboard::find_king_moves(gentype gt)
{
  uint64_t king_moves = adjust_pattern(king_pattern, _own->King);
  switch (gt)
  {
    case gentype::all:
      king_moves &= ~_own->pieces;
      while (king_moves)
      {
        uint64_t to_square = popright_square(king_moves);
        if (!square_is_threatened(to_square, true))
        {
          // Found a valid King move
          if (to_square & _other->pieces)
            add_move(piecetype::King, move_props_capture, _own->King, to_square);
          else
            _movelist.push_back(BitMove(piecetype::King, move_props_none, _own->King, to_square));
        }
      }
      break;
    case gentype::captures:
      king_moves &= _other->pieces;
      while (king_moves)
      {
        uint64_t to_square = popright_square(king_moves);
        if (!square_is_threatened(to_square, true))
          add_move(piecetype::King, move_props_capture, _own->King, to_square);
      }
      break;
    default:
      std::cerr << "Error: unknown unknown move generation type." << std::endl;
  }
}

void Bitboard::find_legal_moves(gentype gt)
{
  init_piece_state();
  assert((_own->pieces & _other->pieces) == zero);
  find_king_moves(gt);
  find_checkers_and_pinned_pieces();
  int n_checkers = std::popcount(_checkers);
  if (n_checkers > 1)
  {
    // It's a double-check. We're finished.
    // Only King-moves are possible and we've
    // already figured them out.
    sort_moves(_movelist);
    return;
  }
  else if (n_checkers == 1)
  {
    // If it's a single check, we must see if
    // the checking piece can be taken or if we
    // can put a piece in between that piece
    // and the King.
    find_moves_after_check(gt);
    sort_moves(_movelist);
    return;
  }
  else
  {
    // Not a check. We're free to use all our
    // pieces, though some of them may be pinned
    // of course.
    find_normal_legal_moves(gt);
  }
  sort_moves(_movelist);
}

inline float Bitboard::get_piece_value(piecetype p_type) const
{
  switch (p_type)
  {
    case piecetype::Pawn:
      return 1.0F;
    case piecetype::Knight:
      case piecetype::Bishop:
      return 3.0F;
    case piecetype::Rook:
      return 5.0F;
    case piecetype::Queen:
      return 9.0F;
    default:
      return 0.0F;
  }
}

inline float Bitboard::get_piece_value(uint64_t square) const
{
  assert(std::has_single_bit(square));

  if ((_white_pieces.Pawns & square) || (_black_pieces.Pawns & square))
  {
    return get_piece_value(piecetype::Pawn);
  }
  else if ((_white_pieces.Queens & square) || (_black_pieces.Queens & square))
  {
    return get_piece_value(piecetype::Queen);
  }
  else if ((_white_pieces.Rooks & square) || (_black_pieces.Rooks & square))
  {
    return get_piece_value(piecetype::Rook);
  }
  else if ((_white_pieces.Bishops & square) || (_black_pieces.Bishops & square))
  {
    return get_piece_value(piecetype::Bishop);
  }
  else if ((_white_pieces.Knights & square) || (_black_pieces.Knights & square))
  {
    return get_piece_value(piecetype::Knight);
  }
  else if ((_white_pieces.King & square) || (_black_pieces.King & square))
  {
    return get_piece_value(piecetype::King);
  }
  return 0.0F;
}

// Moves which are captures or promotions get sorted according to
// a move-ordering scheme. (The evaluation-field is used for keeping
// the value, even if that field was intentionally thought for the
// searh-rutine). The reason is to gain time by bigger cut-offs in
// the search algorithm's pruning function.
// A pawn taking a queen evaluates higher than a Queen taking a pawn.
// ..................................................................
// I add the value of a Queen (just to get a positive number),
// then subtract the value of my own piece and add the value of the
// victim. And I do about the same thing for promotions.
// Lowest value will be one, to distinguish the moves from other
// normal moves with value zero.
// ..................................................................
inline void Bitboard::add_move(piecetype p_type,
                               uint16_t move_props,
                               uint64_t from_square,
                               uint64_t to_square,
                               piecetype promotion_p_type)
{
  assert(move_props != move_props_none && ((move_props & move_props_castling) == 0));
  if (move_props == move_props_none)
  {
    _movelist.push_back(BitMove(p_type, move_props, from_square, to_square));
  }
  else
  {
    // Calculate the value (for later move-ordering) of captures and promotions.
    float eval = 0.0F;
    BitMove move(p_type, move_props, from_square, to_square, promotion_p_type);
    if (move_props & move_props_promotion)
    {
      eval = 9.0F + get_piece_value(promotion_p_type) - 1.0F;
    }
    if (move_props & move_props_capture)
    {
      if (move_props & move_props_en_passant)
        eval = 9.0F;
      else
        eval = 9.0F - get_piece_value(p_type) + get_piece_value(to_square);
    }
    move.evaluation(eval);
    _movelist.push_front(move);
  }
}

inline void Bitboard::sort_moves(std::deque<BitMove>& movelist)
{
  std::deque<BitMove>::iterator end_it;
  for (end_it = movelist.begin(); end_it != movelist.end(); end_it++)
  {
    if (std::abs(end_it->_evaluation) < 0.000000001)
      break;
  }
  if (end_it != movelist.begin())
    std::stable_sort(movelist.begin(), end_it);
}

//inline void Bitboard::sort_moves(std::deque<BitMove>& movelist)
//{
//  unsigned int stop_idx;
//  BitMove tmp_move;
//  for (stop_idx = 0; stop_idx < movelist.size(); stop_idx++)
//  {
//    if (std::abs(movelist[stop_idx]._evaluation) < 0.000000001)
//      break;
//  }
//  unsigned int start_idx = 0;
//  while (true)
//  {
//    bool sorted = true;
//    for (unsigned int i = start_idx; i < stop_idx - 1; i++)
//    {
//      if (movelist[i + 1]._evaluation > movelist[i]._evaluation)
//      {
//        tmp_move = movelist[i];
//        movelist[i] = movelist[i + 1];
//        movelist[i + 1] = tmp_move;
//        sorted = false;
//      }
//    }
//    if (sorted)
//      break;
//    else
//      start_idx++;
//  }
//}

}// End namespace C2_chess

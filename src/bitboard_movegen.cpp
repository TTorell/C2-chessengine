/*
 * bitboard_movegen.cpp
 *
 *  Created on: 11 nov. 2021
 *      Author: torsten
 */
#include "bitboard.hpp"
#include "chesstypes.hpp"
#include "zobrist_bitboard_hash.hpp"

namespace C2_chess
{


void Bitboard::find_long_castling()
{
  uint64_t King_initial_square = (_col_to_move == col::white) ? e1_square : e8_square;
  if (_castling_rights & ((_col_to_move == col::white) ? castling_right_WQ : castling_right_BQ))
  {
    uint64_t castling_empty_squares = (castling_empty_squares_Q & ((_col_to_move == col::white) ? lower_board_half : upper_board_half));
    if (castling_empty_squares & _s.all_pieces)
      return;
    while (castling_empty_squares)
    {
      if (square_is_threatened(popright_square(castling_empty_squares), false))
        return;
    }
    _movelist.push_front(BitMove(piecetype::King, move_props_castling, King_initial_square, King_initial_square << 2));
  }
}

void Bitboard::find_short_castling()
{
  uint64_t King_initial_square = (_col_to_move == col::white) ? e1_square : e8_square;
  if (_castling_rights & ((_col_to_move == col::white) ? castling_right_WK : castling_right_BK))
  {
    uint64_t castling_empty_squares = (castling_empty_squares_K & ((_col_to_move == col::white) ? lower_board_half : upper_board_half));
    if (castling_empty_squares & _s.all_pieces)
      return;
    while (castling_empty_squares)
    {
      if (square_is_threatened(popright_square(castling_empty_squares), false))
        return;
    }
    _movelist.push_front(BitMove(piecetype::King, move_props_castling, King_initial_square, King_initial_square >> 2));
  }
}

void Bitboard::find_Queen_Rook_and_Bishop_moves()
{
  uint64_t from_square;
  uint64_t to_square;
  uint64_t legal_squares = zero;
  uint64_t pieces = (_s.Queens | _s.Rooks | _s.Bishops) & ~_s.pinned_pieces;
  while (pieces)
  {
    from_square = popright_square(pieces);
    piecetype p_type = (from_square & _s.Queens) ? piecetype::Queen :
                       (from_square & _s.Rooks) ? piecetype::Rook : piecetype::Bishop;
    if (p_type != piecetype::Bishop)
    {
      legal_squares |= find_legal_squares(from_square, file[file_idx(from_square)]);
      legal_squares |= find_legal_squares(from_square, rank[rank_idx(from_square)]);
    }
    if (p_type != piecetype::Rook)
    {
      legal_squares |= find_legal_squares(from_square, to_diagonal(from_square));
      legal_squares |= find_legal_squares(from_square, to_anti_diagonal(from_square));
    }
    while (legal_squares)
    {
      to_square = popright_square(legal_squares);
      if (to_square & _s.other_pieces)
        _movelist.push_front(BitMove(p_type, move_props_capture, from_square, to_square));
      else
        _movelist.push_back(BitMove(p_type, move_props_none, from_square, to_square));
    }
  }
}

void Bitboard::find_legal_moves_for_pinned_pieces()
{
  assert(_s.pinned_pieces & ~_s.Pawns); // Pinned Pawns has been taken care of.
  uint64_t from_square;
  uint64_t to_square;
  uint64_t legal_squares = zero;
  uint64_t King_file_idx = file_idx(_s.King);
  uint64_t King_rank_idx = rank_idx(_s.King);
  uint64_t pieces = (_s.Queens | _s.Rooks | _s.Bishops) & _s.pinned_pieces;
  while (pieces)
  {
    from_square = popright_square(pieces);
    piecetype p_type = (from_square & _s.Queens) ? piecetype::Queen :
                       (from_square & _s.Rooks) ? piecetype::Rook : piecetype::Bishop;

    // Here we know that "from_square" is somewhere between our King and a piece of other color.
    if (p_type != piecetype::Bishop)
    {
      if (from_square & file[King_file_idx])
        legal_squares |= find_legal_squares(from_square, file[King_file_idx]);
      else if (from_square & rank[King_rank_idx])
        legal_squares |= find_legal_squares(from_square, rank[King_rank_idx]);
    }
    if (p_type != piecetype::Rook)
    {
      if (from_square & to_diagonal(King_file_idx, King_rank_idx))
        legal_squares |= find_legal_squares(from_square, to_diagonal(King_file_idx, King_rank_idx));
      else if (from_square & to_anti_diagonal(King_file_idx, King_rank_idx))
        legal_squares |= find_legal_squares(from_square, to_anti_diagonal(King_file_idx, King_rank_idx));
    }
    while (legal_squares)
    {
      to_square = popright_square(legal_squares);
      if (to_square & _s.other_pieces)
        _movelist.push_front(BitMove(p_type, move_props_capture, from_square, to_square));
      else
        _movelist.push_back(BitMove(p_type, move_props_none, from_square, to_square));
    }
  }
}

void Bitboard::find_Knight_moves()
{
  uint64_t from_square;
  uint64_t to_square;
  uint64_t to_squares;
  if (_s.Knights)
  {
    uint64_t knights = _s.Knights & ~_s.pinned_pieces;
    while (knights)
    {
      from_square = popright_square(knights);
      to_squares = adjust_pattern(knight_pattern, from_square) & ~_s.own_pieces;
      while (to_squares)
      {
        to_square = popright_square(to_squares);
        if (to_square & _s.other_pieces)
          _movelist.push_front(BitMove(piecetype::Knight, move_props_capture, from_square, to_square));
        else
          _movelist.push_back(BitMove(piecetype::Knight, move_props_none, from_square, to_square));
      }
    }
  }
}

void Bitboard::find_Pawn_moves()
{
  uint64_t to_square;
  uint64_t moved_pawns;
  uint64_t King_file_idx = file_idx(_s.King);
  uint64_t King_rank_idx = rank_idx(_s.King);
  uint64_t King_file = file[King_file_idx];
  // uint64_t King_rank = rank[King_rank_idx];
  uint64_t King_diagonal = to_diagonal(King_file_idx, King_rank_idx);
  uint64_t King_anti_diagonal = to_anti_diagonal(King_file_idx, King_rank_idx);

  uint64_t pawns = _s.Pawns & ~(_s.pinned_pieces & ~King_file);
  // Shift chosen pawns one square ahead. Check if the new square is empty:
  pawns = ((_col_to_move == col::white) ? pawns >> 8 : pawns << 8) & ~_s.all_pieces;
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
    moved_pawns = (~_s.all_pieces) & ((_col_to_move == col::white) ? ((moved_pawns >> 8) & rank[4]) :
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
  // Check for pawn captures and ep.
  pawns = _s.Pawns & ~(_s.pinned_pieces & ~King_diagonal);
  moved_pawns = (_col_to_move == col::white) ? (pawns >> 9) & not_a_file : (pawns << 9) & not_h_file;
  while (moved_pawns)
  {
    to_square = popright_square(moved_pawns);
    if (to_square & _s.other_pieces)
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 9 :
                                                                   to_square >> 9,
                                    to_square);
    else if (to_square == _ep_square)
      try_adding_ep_pawn_move((_col_to_move == col::white) ? to_square << 9 : to_square >> 9);
  }
  pawns = _s.Pawns & ~(_s.pinned_pieces & ~King_anti_diagonal);
  moved_pawns = (_col_to_move == col::white) ? (pawns >> 7) & not_h_file : (pawns << 7) & not_a_file;
  while (moved_pawns)
  {
    to_square = popright_square(moved_pawns);
    if (to_square & _s.other_pieces)
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 7 :
                                                                   to_square >> 7,
                                    to_square);
    else if (to_square == _ep_square)
      try_adding_ep_pawn_move((_col_to_move == col::white) ? to_square << 7 : to_square >> 7);
  }
}

void Bitboard::find_normal_legal_moves()
{

  find_Pawn_moves();
  find_Knight_moves();
  if (_s.pinned_pieces & ~_s.Pawns)
    find_legal_moves_for_pinned_pieces();
  find_Queen_Rook_and_Bishop_moves();
  if ((_col_to_move == col::white && _s.King == e1_square) ||
      (_col_to_move == col::black && _s.King == e8_square))
  {
    find_short_castling();
    find_long_castling();
  }
  // Other King-moves have already been taken care of.
}

void Bitboard::find_Knight_moves_to_square(const uint64_t to_square)
{
  assert(to_square & ~_s.own_pieces);
  if (_s.Knights)
  {
    uint64_t knights = adjust_pattern(knight_pattern, to_square) & _s.Knights;
    while (knights)
    {
      uint64_t knight = popright_square(knights);
      if (knight & ~_s.pinned_pieces)
      {
        if (to_square & _s.other_pieces)
          _movelist.push_front(BitMove(piecetype::Knight, move_props_capture, knight, to_square));
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
  uint64_t other_Queens_or_Rooks = _s.other_Queens | _s.other_Rooks;
  uint64_t other_Queens_or_Bishops = _s.other_Queens | _s.other_Bishops;
// Check file and rank
  if (other_Queens_or_Rooks)
  {
    uint64_t King_ortogonal_squares = ortogonal_squares(_s.King);
    if (King_ortogonal_squares & other_pawn_square)
    {
      possible_pinners = King_ortogonal_squares & other_Queens_or_Rooks;
      while (possible_pinners)
      {
        possible_pinner = popright_square(possible_pinners);
        between_squares = between(_s.King, possible_pinner, King_ortogonal_squares);
        if (between_squares & other_pawn_square)
        {
          if (std::has_single_bit(between_squares & _s.all_pieces))
            return true;
          // own_pawn_square can be a blocker on a rank together with other_pawn_square
          // which both would be gone from the rank after the ep-move.
          if (rank[rank_idx(_s.King)] & other_pawn_square)
          {
            if ((std::popcount(between_squares & _s.all_pieces) == 2) && (between_squares & own_pawn_square))
            {
              return true;
            }
          }
        }
      }
    }
  }
  if (other_Queens_or_Bishops)
  {
    uint64_t King_diagonal_squares = diagonal_squares(_s.King);
    if (King_diagonal_squares & other_pawn_square)
    {
      possible_pinners = King_diagonal_squares & other_Queens_or_Bishops;
      while (possible_pinners)
      {
        possible_pinner = popright_square(possible_pinners);
        between_squares = between(_s.King, possible_pinner, King_diagonal_squares, true);
        if (between_squares & other_pawn_square)
        {
          if (std::has_single_bit(between_squares & _s.all_pieces))
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

  if (from_square & ~_s.pinned_pieces)
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
    if ((other_pawn_square & _s.King_file) == zero)
    {
      if (check_if_other_pawn_is_pinned_ep(other_pawn_square, from_square))
        return;
    }
    _movelist.push_front(BitMove(piecetype::Pawn,
                                 move_props_capture | move_props_en_passant,
                                 from_square,
                                 _ep_square));
  }
  else
  {
// Our own pawn is pinned, but can still capture e.p. along the
// diagonal of the king.
    if (_ep_square & (_s.King_diagonal | _s.King_anti_diagonal))
    {
      // No need to check if other_pawn is pinned, it can't be.
      _movelist.push_front(BitMove(piecetype::Pawn,
                                   move_props_capture | move_props_en_passant,
                                   from_square,
                                   _ep_square));
    }
  }
}

// - The location of from_square and to_square should allow some kind of
//   pawn-move from from_square to to_square.

// Adding Pawn move considering promotion of the Pawn.
void Bitboard::add_pawn_move_check_promotion(uint64_t from_square,
                                             uint64_t to_square)
{
  assert(std::has_single_bit(from_square) && (from_square & _s.Pawns) &&
         std::has_single_bit(to_square));

  if ((_col_to_move == col::white) ? to_square & not_row_8 : to_square & not_row_1)
  {
    if (to_square & ~_s.all_pieces)
    {
      // To empty squares we only allow "straight" pawn-moves. (or e.p., already taken care of)
      if (to_square == (_col_to_move == col::white) ? from_square >> 8 : from_square << 8)
        _movelist.push_back(BitMove(piecetype::Pawn,
                                    move_props_none,
                                    from_square,
                                    to_square));
    }
    else if (to_square & _s.other_pieces)
    {
      _movelist.push_front(BitMove(piecetype::Pawn,
                                   move_props_capture,
                                   from_square,
                                   to_square));
    }
  }
  else
  {
    // Promotion
    uint8_t move_props = move_props_promotion;
    if (to_square & ~_s.all_pieces)
    {
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Bishop));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Rook));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Knight));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square)); // Queen
    }
    else if (to_square & _s.other_pieces)
    {
      move_props |= move_props_capture;
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Bishop));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Rook));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Knight));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square)); // Queen
    }
  }
}

// preconditions:
// - to_square should be empty
void Bitboard::find_pawn_moves_to_empty_square(uint64_t to_square)
{
  assert(std::has_single_bit(to_square) && (to_square & ~_s.all_pieces));
  uint64_t from_square;

  from_square = (_col_to_move == col::white) ? to_square << 8 : to_square >> 8; // one step
  // No pawn moves possible if from_sqare doesn't contain a pawn of
  // color _col_to_move, which isn't pinned.
  if ((from_square & _s.Pawns) && (from_square & ~_s.pinned_pieces))
  {
    add_pawn_move_check_promotion(from_square, to_square);
  }
  else if ((from_square & ~_s.all_pieces) && (to_square & ((_col_to_move == col::white) ? row_4 : row_5)))
  {
    // No blocking piece and correct row for a for two-squares-pawn-move
    (_col_to_move == col::white) ? from_square <<= 8 : from_square >>= 8; // two steps
    if ((from_square & _s.Pawns) && (from_square & ~_s.pinned_pieces))
    {
      _movelist.push_back(BitMove(piecetype::Pawn,
                                  move_props_none,
                                  from_square,
                                  to_square));
    }
  }
  // Normal pawn captures are not possible since to_square is empty.
  // En passant, however, is possible.
  if (to_square & _ep_square)
  {
    if (_ep_square & not_h_file)
    {
      from_square = (_col_to_move == col::white) ? _ep_square << 7 : _ep_square >> 9;
      if (from_square & _s.Pawns)
        try_adding_ep_pawn_move(from_square);
    }
    if (_ep_square & not_a_file)
    {
      from_square = (_col_to_move == col::white) ? _ep_square << 9 : _ep_square >> 7;
      if (from_square & _s.Pawns)
        try_adding_ep_pawn_move(from_square);
    }
  }
}

// The following method doesn't consider possible King-moves to square
void Bitboard::find_moves_to_square(uint64_t to_square)
{
  assert((to_square & ~_s.own_pieces) && std::has_single_bit(to_square));
  uint64_t from_square;
  uint64_t move_candidates;
  uint64_t move_candidate;
  uint64_t between_squares;
  if (to_square & ~_s.all_pieces)
  {
    find_pawn_moves_to_empty_square(to_square);
  }
  else if (to_square & _s.other_pieces)
  {
    // Pawn Captures (e.p. not possible)
    if (to_square & not_h_file)
    {
      from_square = (_col_to_move == col::white) ? to_square << 7 : to_square >> 9;
      if ((from_square & _s.Pawns) && (from_square & ~_s.pinned_pieces))
        add_pawn_move_check_promotion(from_square, to_square);
    }
    if (to_square & not_a_file)
    {
      from_square = (_col_to_move == col::white) ? to_square << 9 : to_square >> 7;
      if ((from_square & _s.Pawns) && (from_square & ~_s.pinned_pieces))
        add_pawn_move_check_promotion(from_square, to_square);
    }
  }

  find_Knight_moves_to_square(to_square);

  // Check file and rank
  uint64_t Queens_or_Rooks = _s.Queens | _s.Rooks;
  if (Queens_or_Rooks)
  {
    uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
    move_candidates = to_ortogonal_squares & Queens_or_Rooks;
    while (move_candidates)
    {
      move_candidate = popright_square(move_candidates);
      if (move_candidate & ~_s.pinned_pieces)
      {
        between_squares = between(to_square, move_candidate, to_ortogonal_squares);
        if ((between_squares & _s.all_pieces) == zero)
        {
          piecetype p_type = (to_square & _s.Queens) ? piecetype::Queen : piecetype::Rook;
          if (to_square & ~_s.all_pieces)
            _movelist.push_back(BitMove(p_type, move_props_none, move_candidate, to_square));
          else if (to_square & _s.other_pieces)
            _movelist.push_front(BitMove(p_type, move_props_capture, move_candidate, to_square));
        }
      }
    }
  }

  // Check diagonals
  uint64_t Queens_or_Bishops = _s.Queens | _s.Bishops;
  if (Queens_or_Bishops)
  {
    uint64_t to_diagonal_squares = diagonal_squares(to_square);
    move_candidates = to_diagonal_squares & Queens_or_Bishops;
    while (move_candidates)
    {
      move_candidate = popright_square(move_candidates);
      if (move_candidate & ~_s.pinned_pieces)
      {
        between_squares = between(to_square, move_candidate, to_diagonal_squares, true); // true = diagonal
        if ((between_squares & _s.all_pieces) == zero)
        {
          piecetype p_type = (to_square & _s.Queens) ? piecetype::Queen : piecetype::Bishop;
          if (to_square & ~_s.all_pieces)
            _movelist.push_back(BitMove(p_type, move_props_none, move_candidate, to_square));
          else if (to_square & _s.other_pieces)
            _movelist.push_front(BitMove(p_type, move_props_capture, move_candidate, to_square));
        }
      }
    }
  }
}

void Bitboard::find_moves_after_check(uint64_t checker)
{
  // All possible King-moves have already been found.
  uint64_t between_squares;
  uint64_t between_square;
  uint64_t King_ortogonal_squares = ortogonal_squares(_s.King);
  if (checker & King_ortogonal_squares)
  {
    // Can we put any piece between the King and the checker?
    between_squares = between(_s.King, checker, King_ortogonal_squares);
    while (between_squares)
    {
      between_square = popright_square(between_squares);
      find_moves_to_square(between_square);
    }
  }
  else
  {
    uint64_t King_diagonal_squares = diagonal_squares(_s.King);
    if (checker & King_diagonal_squares)
    {
      // Can we put any piece between the King and the checker?
      between_squares = between(_s.King, checker, King_diagonal_squares, true);
      while (between_squares)
      {
        between_square = popright_square(between_squares);
        find_moves_to_square(between_square);
      }
    }
  }
// Can we take the checking piece?
  find_moves_to_square(checker);

// Also check if the checker is a Pawn which can be taken e.p.
  if (_ep_square & ((_col_to_move == col::white) ? checker >> 8 : checker << 8))
  {
    if (_ep_square & not_a_file)
    {
      uint64_t from_square = (_col_to_move == col::white) ? _ep_square << 9 : _ep_square >> 7;
      if (from_square & _s.Pawns)
        try_adding_ep_pawn_move(from_square);
    }
    if (_ep_square & not_h_file)
    {
      uint64_t from_square = (_col_to_move == col::white) ? _ep_square << 7 : _ep_square >> 9;
      if (from_square & _s.Pawns)
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
  uint8_t King_file_idx = file_idx(_s.King);
  uint64_t other_Queens_or_Rooks = _s.other_Queens | _s.other_Rooks;
  uint64_t other_Queens_or_Bishops = _s.other_Queens | _s.other_Bishops;

  // Check Pawn-threats
  if (_s.other_Pawns)
  {
    if (King_file_idx != h)
    {
      possible_checker = (_col_to_move == col::white) ? _s.King >> 9 : _s.King << 7;
      if (possible_checker & _s.other_Pawns)
        _s.checkers |= possible_checker;
    }
    if (King_file_idx != a)
    {
      possible_checker = (_col_to_move == col::white) ? _s.King >> 7 : _s.King << 9;
      if (possible_checker & _s.other_Pawns)
        _s.checkers |= possible_checker;
    }
  }

  // Check Knight threats
  if (_s.other_Knights)
    _s.checkers |= (adjust_pattern(knight_pattern, _s.King) & _s.other_Knights);

  // Check threats on file and rank
  if (other_Queens_or_Rooks)
  {
    uint64_t King_ortogonal_squares = ortogonal_squares(_s.King);
    possible_checkers = King_ortogonal_squares & other_Queens_or_Rooks;
    while (possible_checkers)
    {
      possible_checker = popright_square(possible_checkers);
      between_squares = between(_s.King, possible_checker, King_ortogonal_squares);
      if ((between_squares & _s.all_pieces) == zero)
        _s.checkers |= possible_checker;
      else if (std::has_single_bit(between_squares & _s.all_pieces))
      {
        _s.pinned_pieces |= (between_squares & _s.own_pieces);
        _s.pinners |= possible_checker;
      }
    }
  }

  // Check diagonal threats
  if (other_Queens_or_Bishops)
  {
    uint64_t King_diagonal_squares = diagonal_squares(_s.King);
    possible_checkers = King_diagonal_squares & other_Queens_or_Bishops;
    while (possible_checkers)
    {
      possible_checker = popright_square(possible_checkers);
      between_squares = between(_s.King, possible_checker, King_diagonal_squares, true); // true = diagonal
      if ((between_squares & _s.all_pieces) == zero)
        _s.checkers |= possible_checker;
      else if (std::has_single_bit(between_squares & _s.all_pieces))
      {
        _s.pinned_pieces |= (between_squares & _s.own_pieces);
        _s.pinners |= possible_checker;
      }
    }
  }
}

bool Bitboard::square_is_threatened(uint64_t to_square, bool King_is_asking)
{
  uint64_t possible_attackers;
  uint64_t attacker;
  uint64_t tmp_all_pieces = _s.all_pieces;
  uint8_t f_idx = file_idx(to_square);

  // Check Pawn-threats
  if (_s.other_Pawns)
  {
    if ((f_idx != h) && (_s.other_Pawns & ((_col_to_move == col::white) ? to_square >> 9 : to_square << 7)))
      return true;
    if ((f_idx != a) && (_s.other_Pawns & ((_col_to_move == col::white) ? to_square >> 7 : to_square << 9)))
      return true;
  }

  // Check Knight-threats
  if (_s.other_Knights && ((adjust_pattern(knight_pattern, to_square) & _s.other_Knights)))
    return true;

  // Check King (and adjacent Queen-threats)
  if (adjust_pattern(king_pattern, to_square) & (_s.other_King | _s.other_Queens))
    return true;

  if (King_is_asking)
  {
    // Treat King-square as empty to catch xray-attacks through the King-square
    tmp_all_pieces ^= _s.King;
  }

  // Check threats on file and rank
  if (_s.other_Queens | _s.other_Rooks)
  {
    // Check threats on file and rank
    uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
    possible_attackers = to_ortogonal_squares & (_s.other_Queens | _s.other_Rooks);
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_ortogonal_squares) & tmp_all_pieces) == zero)
        return true;
    }
  }

  // Check diagonal threats
  if (_s.other_Queens | _s.other_Bishops)
  {
    uint64_t to_diagonal_squares = diagonal_squares(to_square);
    possible_attackers = to_diagonal_squares & (_s.other_Queens | _s.other_Bishops);
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_diagonal_squares, true) & tmp_all_pieces) == zero)
        return true;
    }
  }
  return false;
}

// Finds normal King-moves not including castling.
inline void Bitboard::find_king_moves()
{
  uint64_t king_moves = adjust_pattern(king_pattern, _s.King);
  king_moves &= ~_s.own_pieces;
  while (king_moves)
  {
    uint64_t to_square = popright_square(king_moves);
    if (!square_is_threatened(to_square, true))
    {
      // Found a valid King move
      if (to_square & _s.other_pieces)
      {
        _movelist.push_front(BitMove(piecetype::King, move_props_capture, _s.King, to_square));
      }
      else
        _movelist.push_back(BitMove(piecetype::King, move_props_none, _s.King, to_square));
    }
  }
}

void Bitboard::find_all_legal_moves()
{
  init_piece_state();
  find_king_moves();
  find_checkers_and_pinned_pieces();
  int n_checkers = std::popcount(_s.checkers);
  if (n_checkers > 1)
  {
    // It's a double-check. We're finished.
    // Only King-moves are possible and we've
    // already figured them out.
    return;
  }
  else if (n_checkers == 1)
  {
    // If it's a single check, we must see if
    // the checking piece can be taken or if we
    // can put a piece in between that piece
    // and the King.
    find_moves_after_check(_s.checkers);
    return;
  }
  else
  {
    // Not a check. We're free to use all our
    // pieces, though some of them may be pinned
    // of course.
    find_normal_legal_moves();
  }
}

uint64_t Bitboard::find_legal_squares(uint64_t sq, uint64_t mask)
{
  mask ^= sq;
  uint64_t left_side = mask & _s.all_pieces & ~(sq - 1);
  uint64_t left_blocker = rightmost_square(left_side);
  uint64_t right_side = mask & _s.all_pieces & (sq - 1);
  uint64_t right_blocker = leftmost_square(right_side);
  uint64_t other_color_blockers = (left_blocker | right_blocker) & _s.other_pieces;
  if (right_blocker)
    return (((left_blocker - one) ^ ((right_blocker << 1) - one)) | other_color_blockers) & mask;
  else
    return ((left_blocker - one) | other_color_blockers) & mask;
}



} // End namespace C2_chess
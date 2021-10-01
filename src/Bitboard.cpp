//============================================================================
// Name        : Bitboard.cpp
// Author      : Torsten
// Version     :
// Copyright   : Your copyright notice
// Description : C++ code for a bitboard-representation of a chess board.
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include "chessfuncs.hpp"
#include "Bitboard.hpp"
#include "zobrist_bitboard_hash.hpp"

namespace C2_chess
{

std::atomic<bool> Bitboard::time_left(false);
Bitboard Bitboard::level_boards[38];
Zobrist_bitboard_hash Bitboard::hash_table;

inline void Bitboard::clear_movelist()
{
  _movelist.clear();
}

inline void Bitboard::init_piece_state()
{
  clear_movelist();
  _s.pinned_pieces = 0L;
  _s.checking_piece_square = 0L;
  _s.check_count = 0;

  if (_col_to_move == col::white)
  {
    _s.King = _W_King;
    _s.King_file_index = _W_King_file_index;
    _s.King_rank_index = _W_King_rank_index;
    _s.King_file = _W_King_file;
    _s.King_rank = _W_King_rank;
    _s.King_diagonal = _W_King_diagonal;
    _s.King_anti_diagonal = _W_King_anti_diagonal;
    _s.King_initial_square = e1_square;
    _s.castling_rights_K = _castling_rights & castling_rights_WK;
    _s.castling_rights_Q = _castling_rights & castling_rights_WQ;
    _s.own_pieces = _W_King | _W_Queens | _W_Rooks | _W_Bishops | _W_Knights | _W_Pawns;
    _s.adjacent_files = (_W_King_file_index - 1 >= a) ? file[_W_King_file_index - 1] : 0L;
    _s.adjacent_files |= file[_W_King_file_index + 1];
    _s.r1 = rank[_W_King_rank_index + 1]; // Rank from which white pawns can attack.
    _s.r2 = rank[_W_King_rank_index - 1];
    _s.adjacent_ranks = _s.r1 | _s.r2;
    _s.Queens = _W_Queens;
    _s.Rooks = _W_Rooks;
    _s.Bishops = _W_Bishops;
    _s.Knights = _W_Knights;
    _s.Pawns = _W_Pawns;
    _s.pawnstep = 1;

    _s.other_Queens = _B_Queens;
    _s.other_Queens_or_Rooks = _B_Queens | _B_Rooks;
    _s.other_Queens_or_Bishops = _B_Queens | _B_Bishops;
    _s.other_King_Queens_or_Bishops = _B_King | _s.other_Queens_or_Bishops;
    _s.other_King_Queens_or_Rooks = _B_King | _s.other_Queens_or_Rooks;
    _s.other_Rooks = _B_Rooks;
    _s.other_Bishops = _B_Bishops;
    _s.other_Knights = _B_Knights;
    _s.other_Pawns = _B_Pawns;
    _s.other_pieces = _s.other_King_Queens_or_Rooks | _B_Bishops | _B_Knights | _B_Pawns;
    _s.all_pieces = _s.own_pieces | _s.other_pieces;
    _s.empty_squares = whole_board ^ _s.all_pieces;
  }
  else
  {
    // col_to_move is black
    _s.King = _B_King;
    _s.King_file_index = _B_King_file_index;
    _s.King_rank_index = _B_King_rank_index;
    _s.King_file = _B_King_file;
    _s.King_rank = _B_King_rank;
    _s.King_diagonal = _B_King_diagonal;
    _s.King_anti_diagonal = _B_King_anti_diagonal;
    _s.King_initial_square = e1_square;
    _s.castling_rights_K = _castling_rights & castling_rights_BK;
    _s.castling_rights_Q = _castling_rights & castling_rights_BQ;
    _s.own_pieces = _B_King | _B_Queens | _B_Rooks | _B_Bishops | _B_Knights | _B_Pawns;
    _s.adjacent_files = (_B_King_file_index - 1 >= a) ? file[_B_King_file_index - 1] : 0L;
    _s.adjacent_files |= file[_B_King_file_index + 1];
    _s.r1 = rank[_B_King_rank_index - 1]; // Rank from which white pawns can attack.
    _s.r2 = rank[_B_King_rank_index + 1];
    _s.adjacent_ranks = _s.r1 | _s.r2;
    _s.Queens = _B_Queens;
    _s.Rooks = _B_Rooks;
    _s.Bishops = _B_Bishops;
    _s.Knights = _B_Knights;
    _s.Pawns = _B_Pawns;
    _s.pawnstep = -1;

    _s.other_Queens = _W_Queens;
    _s.other_Queens_or_Rooks = _W_Queens | _W_Rooks;
    _s.other_Queens_or_Bishops = _W_Queens | _W_Bishops;
    _s.other_King_Queens_or_Bishops = _W_King | _s.other_Queens_or_Bishops;
    _s.other_King_Queens_or_Rooks = _W_King | _s.other_Queens_or_Rooks;
    _s.other_Rooks = _W_Rooks;
    _s.other_Bishops = _W_Bishops;
    _s.other_Knights = _W_Knights;
    _s.other_Pawns = _W_Pawns;
    _s.other_pieces = _s.other_King_Queens_or_Rooks | _W_Bishops | _W_Knights | _W_Pawns;
    _s.all_pieces = _s.own_pieces | _s.other_pieces;
    _s.empty_squares = whole_board ^ _s.all_pieces;
  }
}

bool Bitboard::square_is_threatened(int8_t file_index, int8_t rank_index, bool King_is_asking)
{
  // -----------------------------------------------------------
  // Checks if square is threatened by any piece
  // of color "threatened_by_color".
  // -----------------------------------------------------------

  uint64_t square_file = file[file_index];
  uint64_t square_rank = rank[rank_index];
  uint64_t square = square_file & square_rank;

  uint64_t adjacent_files = file[file_index + 1];
  adjacent_files |= ((file_index - 1 >= a) ? file[file_index - 1] : 0L);
  uint64_t adjacent_ranks = rank[rank_index + 1];
  adjacent_ranks |= rank[rank_index - 1];
  uint64_t r1; // Rank from which Pawns can attack the square.
  if (_col_to_move == col::white)
    r1 = rank[rank_index + 1];
  else
    r1 = rank[rank_index - 1];
  // Check Pawns and King and adjacent Queen or Bishop diagonal threats)
  // Check Pawns, King and adjacent Queen or Bishop diagonal threats.
  // White pawns can only attack diagonally from below and
  // black Pawns from above (_s.r1)
  if (adjacent_files & r1 & _s.other_Pawns)
    return true;
  // The other pieces can attack from all diagonals.
  if (adjacent_files & adjacent_ranks & _s.other_King_Queens_or_Bishops)
    return true;

  // Check King, Queen or Rook threats on adjacent squares on the file or rank.
  uint64_t squares = (square_file & adjacent_ranks) | (adjacent_files & square_rank);
  if (squares & _s.other_King_Queens_or_Rooks)
    return true;

  // Check Knights two ranks from square
  uint64_t knight_ranks = rank[rank_index + 2];
  knight_ranks |= (rank_index - 2 >= 1) ? rank[rank_index - 2] : 0L;
  if (adjacent_files & knight_ranks & _s.other_Knights)
    return true;
  // Check Knights two files from square
  uint64_t knight_files = file[file_index + 2];
  knight_files |= (file_index - 2 >= a) ? file[file_index - 2] : 0L;
  if (knight_files & adjacent_ranks & _s.other_Knights)
    return true;

  // Check Queen or Rook on same file or rank
  // If it is the black King asking if he can go to the square, then we can't use
  // the King as a blocking piece and the square will be threatened when the King
  // moves there anyway, so we "sort of" remove the King, before asking. You could
  // say that the square is threatened by x-ray through the King.
  uint64_t other_pieces = _s.other_pieces ^ _s.other_Queens_or_Rooks;
  uint64_t own_pieces = _s.own_pieces;
  if (King_is_asking)
    own_pieces ^= _s.King;
  int8_t f, r;
  if ((square_file ^ square) & _s.other_Queens_or_Rooks)
  {
    // Opponent has at least one Queen or Rook on the same file
    // Step downwards from the square.
    for (r = rank_index - 1; r >= 1; r--)
    {
      if (square_file & rank[r] & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (square_file & rank[r] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // Step upwards from the square.
    for (r = rank_index + 1; r <= 8; r++)
    {
      if (square_file & rank[r] & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (square_file & rank[r] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
  }
  if ((square_rank ^ square) & _s.other_Queens_or_Rooks)
  {
    // Opponent has a Queen or Rook on the same rank.
    // Step to the left from the square.
    for (f = file_index - 1; f >= a; f--)
    {
      if (file[f] & square_rank & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (file[f] & square_rank & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // We haven't found the queen_or rook yet.
    // Step to the right from the square.
    for (f = file_index + 1; f <= h; f++)
    {
      if (file[f] & square_rank & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (file[f] & square_rank & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
  }

  // Check for queen or bishop threats along diagonals.
  // We  can't use bishops as blocking pieces on the
  // diagonals. It would still be check.
  other_pieces = _s.other_pieces ^ _s.other_Queens_or_Bishops;
  if ((diagonal[8 + file_index - rank_index] ^ square) & _s.other_Queens_or_Bishops)
  {
    // Step north-east from the square.
    for (f = file_index + 1, r = rank_index + 1; f <= h && r <= 8; f++, r++)
    {
      if (file[f] & rank[r] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[f] & rank[r] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // Step south-west from the square.
    for (f = file_index - 1, r = rank_index - 1; f >= a && r >= 1; f--, r--)
    {
      if (file[f] & rank[r] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[f] & rank[r] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
  }
  if ((anti_diagonal[file_index + rank_index - 1] ^ square) & _s.other_Queens_or_Bishops)
  {
    // Step north-west from the square.
    for (f = file_index - 1, r = rank_index + 1; f >= a && r <= 8; f--, r++)
    {
      if (file[f] & rank[r] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[f] & rank[r] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // Step south-east from the square.
    for (f = file_index + 1, r = rank_index - 1; f <= h && r >= 1; f++, r--)
    {
      if (file[f] & rank[r] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[f] & rank[r] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
  }
  return false;
}

inline void Bitboard::find_king_moves()
{

  int8_t file_min = std::max(a, _s.King_file_index - 1);
  int8_t file_max = std::min(h, _s.King_file_index + 1);
  int8_t rank_min = std::max(1, _s.King_rank_index - 1);
  int8_t rank_max = std::min(8, _s.King_rank_index + 1);

  for (int8_t f = file_min; f <= file_max; f++)
  {
    for (int8_t r = rank_min; r <= rank_max; r++)
    {
      uint64_t square = file[f] & rank[r];
      // King's own square always contains the King himself.
      if ((square & _s.own_pieces) == 0L)
      {
        // There is NOT one of King's own pieces on the square, continue examination.
        if (!square_is_threatened(f, r, true))
        {
          // Found a valid King move
          if (square & _s.other_pieces)
          {
            _movelist.push_front(BitMove(piecetype::King, move_props_capture, _s.King, square));
          }
          else
            _movelist.push_back(BitMove(piecetype::King, move_props_none, _s.King, square));
        }
      }
    }
  }
}

// Checks one square while stepping out from King's square in any direction.
// returns true if we can stop the loop
// returns false if we must keep on looping
bool Bitboard::find_check_or_pinned_piece(uint64_t square,
                                          uint64_t threatening_pieces,
                                          uint64_t opponents_other_pieces,
                                          uint64_t& pinned_piece)
{
  if (square & opponents_other_pieces)
  {
    // Piece of other color (not threatening piece),
    // no threats or pins possible.
    pinned_piece = 0L;
    return true;
  }
  if (square & threatening_pieces)
  {
    if (pinned_piece == 0L)
    {
      _s.check_count++;
      _s.checking_piece_square = square;
    }
    _s.pinned_pieces |= pinned_piece;
    return true;
  }
  if (square & _s.own_pieces)
  {
    if (pinned_piece) // Two of King's own pieces are in between.
    {
      pinned_piece = 0L;
      return true;
    }
    else
      pinned_piece |= (square); // Not sure if it's pinned yet though
  }
  return false;
}

inline void Bitboard::contains_checking_piece(const uint64_t square,
                                              const uint64_t pieces,
                                              const uint64_t forbidden_files = 0L)
{
  if ((_s.King & forbidden_files) == 0L)
  {
    if (square & pieces)
    {
      _s.checking_piece_square = square;
      _s.check_count++;
    }
  }
}

void Bitboard::look_for_checks_and_pinned_pieces()
{
  // --------------------------------------------------------
  // Detects check and double check and pinned_pieces.
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

//  // ----------------------------------------
//  // Old implementation of pawn-checks
//  // and knight-checks:
//  // (But now we must also tell from which
//  // square the check comes.)
//  // ----------------------------------------
//
//  // Pawn check? (There can only be one pawn-check in a legal position.)
//  if (_s.adjacent_files & _s.r1 & _s.other_Pawns)
//  {
//    check_count++;
//  }
//
//  if (check_count == 0 && _s.other_Knights)
//  {
//    // Check Knights two ranks from square
//    uint64_t knight_ranks = rank[_s.King_rank_index + 2];
//    knight_ranks |= (_s.King_rank_index - 2 >= 1) ? rank[_s.King_rank_index - 2] : 0L;
//    if (_s.adjacent_files & knight_ranks & _s.other_Knights)
//    {
//      check_count++;
//    }
//    else
//    {
//      // Check Knights two files from square
//      uint64_t knight_files = file[_s.King_file_index + 2];
//      knight_files |= (_s.King_file_index - 2 >= a) ? file[_s.King_file_index - 2] : 0L;
//      if (knight_files & _s.adjacent_ranks & _s.other_Knights)
//      {
//        check_count++;
//      }
//    }
//  }

  uint8_t check_count = 0;

  // ----------------------------------------
  // Is there a check from a Pawn?
  // ----------------------------------------
  // Go north-east/south-east.
  if ((_col_to_move == col::white))
    contains_checking_piece(_s.King >> 9, _s.other_Pawns, h_file);
  else
    contains_checking_piece(_s.King << 7, _s.other_Pawns, h_file);
  // Go north-west/south-west.
  if ((_col_to_move == col::white))
    contains_checking_piece(_s.King >> 7, _s.other_Pawns, a_file);
  else
    contains_checking_piece(_s.King << 9, _s.other_Pawns, a_file);

  // ----------------------------------------
  // Knight checks? (Only one knigh-check is
  // possible and it cannot be combined with
  // a pawn-check.)
  // ----------------------------------------
  if (_s.other_Knights && (check_count == 0))
  {
    // Check Knights far east
    contains_checking_piece(_s.King >> 10,
                            _s.other_Knights,
                            h_file | g_file);
    contains_checking_piece(_s.King << 6,
                            _s.other_Knights,
                            h_file | g_file);
    // Check Knights close east
    contains_checking_piece(_s.King >> 17,
                            _s.other_Knights,
                            h_file);
    contains_checking_piece(_s.King << 15,
                            _s.other_Knights,
                            h_file);
    // Check Knights far west
    contains_checking_piece(_s.King >> 6,
                            _s.other_Knights,
                            a_file | b_file);
    contains_checking_piece(_s.King << 10,
                            _s.other_Knights,
                            a_file | b_file);
    // Check Knights close west
    contains_checking_piece(_s.King >> 15,
                            _s.other_Knights,
                            a_file);
    contains_checking_piece(_s.King << 17,
                            _s.other_Knights,
                            a_file);
  }
  // ----------------------------------------
  // Step out from the King in all directions
  // to find checks and pinned pieces.
  // ----------------------------------------
  _s.pinned_pieces = 0L;
  uint64_t pinned_piece = 0L;
  // Check Queen or Rook on same file or rank
  int8_t f, r;
  uint64_t square;
  uint64_t other_pieces = _s.other_pieces ^ _s.other_Queens_or_Rooks;
  if (_s.King_file & _s.other_Queens_or_Rooks)
  {
    // Opponent has a Queen or Rook on the same file
    // Step downwards from the square.
    for (r = _s.King_rank_index - 1; r >= 1; r--)
    {
      square = _s.King_file & rank[r];
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Rooks,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step upwards from the square.
    pinned_piece = 0L;
    for (r = _s.King_rank_index + 1; r <= 8; r++)
    {
      square = _s.King_file & rank[r];
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Rooks,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
  }
  if (_s.King_rank & _s.other_Queens_or_Rooks)
  {
    // Opponent has a Queen or Rook on the same rank
    // Step west from the square.
    pinned_piece = 0L;
    for (f = _s.King_file_index - 1; f >= a; f--)
    {
      square = file[f] & _s.King_rank;
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Rooks,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step east from square
    pinned_piece = 0L;
    for (f = _s.King_file_index + 1; f <= h; f++)
    {
      square = file[f] & _s.King_rank;
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Rooks,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
  }
  // -------------------------------------------------
  // Check for queen or bishop threats along diagonals.
  // (We  can't use white bishops as blocking pieces
  // on the diagonals. It would still be check. But
  // now white rooks can block instead.)
  // -------------------------------------------------
  other_pieces = _s.other_pieces ^ _s.other_Queens_or_Bishops;
  if (_s.King_diagonal & _s.other_Queens_or_Bishops)
  {
    // Step north-east from the square.
    pinned_piece = 0L;
    for (f = _s.King_file_index + 1, r = _s.King_rank_index + 1; f <= h && r <= 8; f++, r++)
    {
      square = file[f] & rank[r];
      // TODO if (square & _All_pieces)
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Bishops,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step south-west from the square.
    pinned_piece = 0L;
    for (f = _s.King_file_index - 1, r = _s.King_rank_index - 1; f >= a && r >= 1; f--, r--)
    {
      square = file[f] & rank[r];
      // TODO if (square & _All_pieces)
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Bishops,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
  }
  if (_s.King_anti_diagonal & _s.other_Queens_or_Bishops)
  {
    // Step north-west from the square.
    pinned_piece = 0L;
    for (f = _s.King_file_index - 1, r = _s.King_rank_index + 1; f >= a && r <= 8; f--, r++)
    {
      square = file[f] & rank[r];
      // TODO if (square & _All_pieces)
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Bishops,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step south-east from the square.
    pinned_piece = 0L;
    for (f = _s.King_file_index + 1, r = _s.King_rank_index - 1; f <= h && r >= 1; f++, r--)
    {
      square = file[f] & rank[r];
      // TODO if (square & _All_pieces)
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Bishops,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
  }
}

void Bitboard::step_from_King_to_pinning_piece(uint64_t from_square,
                                               uint8_t inc,
                                               piecetype p_type,
                                               uint64_t pinning_pieces)
{
  uint64_t to_square;
  // Check if the pinned_piece is to the (east or north of the King)
  // or to the (west or south of the King), so we now which way to shift.
  // Step out from the King in the direction of the pinned and pining pieces.
  for (to_square = ((long int) (_s.King - from_square) > 0) ? _s.King >> inc : _s.King << inc;
      (to_square & pinning_pieces) == 0L;
      ((long int) (_s.King - from_square) > 0) ? to_square >>= inc : to_square <<= inc)
  {
    if (to_square != from_square)
      _movelist.push_back(BitMove(p_type, move_props_none, from_square, to_square));
  }
  // Loop stopped at the square of the pinning piece. So we can just take it.
  _movelist.push_front(BitMove(p_type, move_props_capture, from_square, to_square));
}

void Bitboard::find_legal_moves_for_pinned_piece(uint64_t from_square)
{
  uint64_t to_square;
  // Here we know that "square" is somewhere between our King and a piece of other color.
  if (from_square & _s.King_file)
  {
    // The piece is pinned along the King-file.
    // Only Pawns, Rooks and Queens can move along a file.
    // Can a pawn move one square along the file?
    if (from_square & _s.Pawns)
    {
      // No promotion possible here, since the pawn is pinned along the King-file.
      to_square = (_col_to_move == col::white) ? from_square >> 8 : from_square << 8;
      if (to_square & (_s.empty_squares)) // No blocking piece
      {
        _movelist.push_back(BitMove(piecetype::Pawn,
                                    move_props_none,
                                    from_square,
                                    to_square));
        // Can the pawn move two squares along the file?
        if (from_square & ((_col_to_move == col::white) ? row_2 : row_7))
        {
          (_col_to_move == col::white) ? to_square >>= 8 : to_square <<= 8;
          if (to_square & (_s.empty_squares)) // No blocking piece for two-square-move
            _movelist.push_back(BitMove(piecetype::Pawn,
                                        move_props_none,
                                        from_square,
                                        to_square));
        }
      }
    }

    if (from_square & (_s.Queens))
      step_from_King_to_pinning_piece(from_square, 8, piecetype::Queen, _s.other_Queens_or_Rooks);
    else if (from_square & _s.Rooks)
      step_from_King_to_pinning_piece(from_square, 8, piecetype::Rook, _s.other_Queens_or_Rooks);
    return;
  }
  if (from_square & _s.King_rank)
  {
    // The piece is pinned along the King-rank.
    // Only Rooks and Queens can move along a rank.
    if (from_square & (_s.Queens))
      step_from_King_to_pinning_piece(from_square, 1, piecetype::Queen, _s.other_Queens_or_Rooks);
    else if (from_square & _s.Rooks)
      step_from_King_to_pinning_piece(from_square, 1, piecetype::Rook, _s.other_Queens_or_Rooks);
    return;
  }
  if (from_square & _s.King_diagonal)
  {
    // The piece is pinned along the King-diagonal.
    // Only Queen, Bishop and a capturing Pawn (en_passant included)
    // can move along a diagonal.

    // Can a Pawn take the pinning piece? Or can a pawn take e.p. and still be on the
    // diagonal?
    uint64_t to_square = (_col_to_move == col::white) ? from_square >> 9 : from_square << 9;
    if (from_square & _s.Pawns)
    {
      if (to_square & _ep_square)
      {
        try_adding_ep_pawn_move(from_square);
      }
      else
      {
        if (to_square & _s.other_pieces)
          add_pawn_move_check_promotion(from_square, to_square);
      }
    }
    if (from_square & _s.Queens)
      step_from_King_to_pinning_piece(from_square, 9, piecetype::Queen, _s.other_Queens_or_Bishops);
    else if (from_square & (_s.Bishops))
      step_from_King_to_pinning_piece(from_square, 9, piecetype::Bishop, _s.other_Queens_or_Bishops);
    return;
  }
  if (from_square & _s.King_anti_diagonal)
  {
    // The piece is pinned along the King-antidiagonal.
    // Only Queen, Bishop and a capturing Pawn (en_passant included)
    // can move along an antidiagonal.

    // Can a Pawn take the pinning piece? Or can a pawn take e.p. and still be on the
    // diagonal?
    uint64_t to_square = (_col_to_move == col::white) ? from_square >> 7 : from_square << 7;
    if (from_square & _s.Pawns)
    {
      if (to_square & _ep_square)
      {
        try_adding_ep_pawn_move(from_square);
      }
      else
      {
        if (to_square & _s.other_pieces)
          add_pawn_move_check_promotion(from_square, to_square);
      }
    }
    if (from_square & _s.Queens)
      step_from_King_to_pinning_piece(from_square, 7, piecetype::Queen, _s.other_Queens_or_Bishops);
    else if (from_square & (_s.Bishops))
      step_from_King_to_pinning_piece(from_square, 7, piecetype::Bishop, _s.other_Queens_or_Bishops);
    return;
  }
}

// Inserts move based on the content of to_square and
// if the piece is pinned or not.
// A Move to a square occupied by own piece is ignored.
// If to_square == 0L ("above" or "below" the board)
// no move will be added,
inline void Bitboard::try_adding_move(uint64_t pieces,
                                      piecetype p_type,
                                      uint8_t move_props,
                                      uint64_t from_square,
                                      uint64_t to_square)
{
  if (from_square & pieces & (whole_board ^ _s.pinned_pieces))
  {
    if (to_square & _s.empty_squares)
      _movelist.push_back(BitMove(p_type, move_props, from_square, to_square));
    else if (to_square & _s.other_pieces)
      _movelist.push_front(BitMove(p_type, move_props | move_props_capture, from_square, to_square));
  }
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
  if (from_square & (whole_board ^ _s.pinned_pieces))
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
    if (other_pawn_square & _s.King_rank)
    {
      if (check_if_other_pawn_is_pinned_ep(other_pawn_square, from_square, 1))
        return;
    }
    else if (other_pawn_square & _s.King_diagonal)
    {
      if (check_if_other_pawn_is_pinned_ep(other_pawn_square, from_square, 9))
        return;
    }
    else if (other_pawn_square & _s.King_anti_diagonal)
    {
      if (check_if_other_pawn_is_pinned_ep(other_pawn_square, from_square, 7))
        return;
    }
    _movelist.push_front(BitMove(piecetype::Pawn,
                                 move_props_capture | move_props_en_passant,
                                 from_square,
                                 _ep_square));
  }
  else if ((from_square & _s.Pawns) && (from_square & _s.pinned_pieces))
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

// Adding Pawn move considering promotion of the Pawn.
// These preconditions must be fulfilled:
// - from_square must contain a pawn of color
//   _col_to_move.
// - It must not be pinned on a rank.
// - If it's a capture (this would mean capturing the pinning piece)
//   or e.p. it may be pinned on the the same diagonal which it moves
//   along.
// - If it's not a capture or e.p. it may be pinned on the
//   King-file only.
// - If _ep_square is set, then that square should of course be empty.
// - The location of from_square and to_square should allow some kind of
//   pawn-move from from_square to to_square.
void Bitboard::add_pawn_move_check_promotion(uint64_t from_square,
                                             uint64_t to_square)
{
  if ((_col_to_move == col::white) ? to_square & not_row_8 : to_square & not_row_1)
  {
    if (to_square & _s.empty_squares)
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
  else if (to_square) // Condition not necessary if we never try to add illegal Pawn moves.
  {
    // Promotion
    uint8_t move_props = move_props_promotion;
    if (to_square & _s.empty_squares)
    {
      _movelist.push_back(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Bishop));
      _movelist.push_back(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Rook));
      _movelist.push_back(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Knight));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square)); // Queen
    }
    else if (to_square & _s.other_pieces)
    {
      move_props |= move_props_capture;
      _movelist.push_back(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Bishop));
      _movelist.push_back(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Rook));
      _movelist.push_back(BitMove(piecetype::Pawn, move_props, from_square, to_square, piecetype::Knight));
      _movelist.push_front(BitMove(piecetype::Pawn, move_props, from_square, to_square)); // Queen
    }
  }
}

// Find all possible moves for a pawn on from_square
// Preconditions:
// - There should be a, not pinned, pawn of color _col_to_move
//   on from_square. (pinned pieces are taken care of elsewhere.)
void Bitboard::find_Pawn_moves(const uint64_t from_square)
{
  uint64_t to_square;
  // Can the Pawn take en_passant or capture piece
  if (from_square & not_h_file)
  {
    to_square = (_col_to_move == col::white) ? from_square >> 9 : from_square << 7; // Go north east or south east.
    if (to_square & _ep_square)
      try_adding_ep_pawn_move(from_square);
    else if (to_square & _s.other_pieces)
      add_pawn_move_check_promotion(from_square, to_square);
  }
  if (from_square & not_a_file)
  {
    to_square = (_col_to_move == col::white) ? from_square >> 7 : from_square << 9; // Go north west or south west.
    if (to_square & _ep_square)
      try_adding_ep_pawn_move(from_square);
    else if (to_square & _s.other_pieces)
      add_pawn_move_check_promotion(from_square, to_square);
  }
  // Can the pawn move one square along the file?
  to_square = (_col_to_move == col::white) ? from_square >> 8 : from_square << 8; // Go one step north or south.
  if (to_square & _s.empty_squares)
  {
    add_pawn_move_check_promotion(from_square, to_square);
    // Can the pawn move two squares along the file?
    if (from_square & ((_col_to_move == col::white) ? row_2 : row_7))
    {
      (_col_to_move == col::white) ? to_square >>= 8 : to_square <<= 8;
      if (to_square & _s.empty_squares)
        _movelist.push_back(BitMove(piecetype::Pawn,
                                    move_props_none,
                                    from_square,
                                    to_square));
    }
  }
}

void Bitboard::find_Knight_moves(const uint64_t from_square)
{
  // Here we must check so the file_index of the knight-move is within the board-limits (a-h).
  // If the rank_index is outside the board, then to_square will be zero, so this is not a
  // problem.

  if (from_square & not_a_b_files)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square >> 6); // -2 + 1*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square << 10); // -2 - 1*8
  }
  if (from_square & not_a_file)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square >> 15); // -1 + 2*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square << 17); // -1 - 2*8
  }
  if (from_square & not_g_h_files)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square >> 10); // 2 + 1*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square << 6); // 2 - 1*8
  }

  if (from_square & not_h_file)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square >> 17); // 1 + 2*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, from_square, from_square << 15); // 1 - 2*8
  }
}

void Bitboard::find_Knight_moves_to_square(const uint64_t to_square)
{
  // Here we must check so the file_index of the knight-move is within the board-limits (a-h).
  // If the rank_index is outside the board, then to_square will be zero, so this is not a
  // problem.
  if (to_square & not_a_b_files)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square >> 6, to_square); // -2 + 1*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square << 10, to_square); // -2 - 1*8
  }
  if (to_square & not_a_file)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square >> 15, to_square); // -1 + 2*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square << 17, to_square); // -1 - 2*8
  }
  if (to_square & not_g_h_files)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square >> 10, to_square); // 2 + 1*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square << 6, to_square); // 2 - 1*8
  }

  if (to_square & not_h_file)
  {
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square >> 17, to_square); // 1 + 2*8
    try_adding_move(_s.Knights, piecetype::Knight, move_props_none, to_square << 15, to_square); // 1 - 2*8
  }
}

void Bitboard::find_Bishop_or_Queen_moves(const uint64_t from_square, const piecetype p_type)
{
  // Go north east
  uint64_t to_square;
  uint64_t pieces = (p_type == piecetype::Queen) ? _s.Queens : _s.Bishops;
  for (to_square = from_square >> 9;
      (to_square & not_a_file);
      to_square >>= 9)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
  // Go south west
  for (to_square = from_square << 9;
      (to_square & not_h_file);
      to_square <<= 9)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
  // Go north west
  for (to_square = from_square >> 7;
      (to_square & not_h_file);
      to_square >>= 7)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
  // Go South est
  for (to_square = from_square << 7;
      (to_square & not_a_file);
      to_square <<= 7)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
}

void Bitboard::find_Rook_or_Queen_moves(const uint64_t from_square, piecetype p_type)
{
  // Go east
  uint64_t to_square;
  uint64_t pieces = (p_type == piecetype::Queen) ? _s.Queens : _s.Rooks;
  for (to_square = from_square >> 1;
      (to_square & not_a_file);
      to_square >>= 1)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
  // Go west
  for (to_square = from_square << 1;
      (to_square & not_h_file);
      to_square <<= 1)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
  // Go North
  for (to_square = from_square >> 8;
      (to_square & whole_board);
      to_square >>= 8)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
  // Go south
  for (to_square = from_square << 8;
      (to_square & whole_board);
      to_square <<= 8)
  {
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
    if (to_square & _s.all_pieces)
      break;
  }
}

// The following method is specific for checking if the squares between the King's
// initial square and the Rook on the king-side are threatened by any of the opponent's
// pieces. It presumes that these squares are empty, so it doesn't check threats from
// the side along the King rank, only from the "front" and from the two "front diagonals".
// Checking threats from "behind" would obviously be pointless, since the squares are
// on the brim of he chess board.)
bool Bitboard::castling_squares_are_threatened_K(const uint64_t square)
{
  uint64_t board_half = lower_board_half;
  uint8_t di_offset = 7;
  uint8_t ad_offset = 0;
  if (_col_to_move == col::black)
  {
    board_half = lower_board_half;
    di_offset = 0;
    ad_offset = 7;
  }

  if ((threatening_pawn_squares_K & board_half) & (_s.other_Pawns | _s.other_King_Queens_or_Bishops))
    return true;
  if ((threatening_knight_squares_K & board_half) & _s.other_Knights)
    return true;

  uint64_t tmp_square = square >> 1;
  for (uint8_t index = 5; index <= 7; index++, tmp_square >>= 1)
  {
    // Is square threatened on file?
    if (file[index] & _s.other_Queens_or_Rooks)
    {
      for (uint64_t sq = (_col_to_move == col::white) ? tmp_square >> 8 : tmp_square <<= 8;
          true;
          (_col_to_move == col::white) ? sq >>= 8 : sq <<= 8)
      {
        if (sq & _s.other_Queens_or_Rooks)
          return true;
        if (sq & (_s.all_pieces))
          break;
      }
    }

    // Is square threatened on diagonal?
    if (diagonal[index + di_offset] & _s.other_Queens_or_Bishops)
    {
      for (uint64_t sq = (_col_to_move == col::white) ? tmp_square >> 9 : tmp_square << 9;
          true;
          (_col_to_move == col::white) ? sq >>= 9 : sq <<= 9)
      {
        if (sq & _s.other_Queens_or_Bishops)
          return true;
        if (sq & _s.all_pieces)
          break;
      }
    }
    // Is square threatened on anti diagonal?
    if (anti_diagonal[index + ad_offset] & _s.other_Queens_or_Bishops)
    {
      for (uint64_t sq = (_col_to_move == col::white) ? tmp_square >> 7 : tmp_square << 7;
          true;
          (_col_to_move == col::white) ? sq >>= 7 : sq <<= 7)
      {
        if (sq & _s.other_Queens_or_Bishops)
          return true;
        if (sq & _s.all_pieces)
          break;
      }
    }
  }
  return false;
}

// This is analog to the previous method, but on the queen-side.
bool Bitboard::castling_squares_are_threatened_Q(const uint64_t square)
{
  uint64_t board_half = lower_board_half;
  uint8_t di_offset = 7;
  uint8_t ad_offset = 0;
  if (_col_to_move == col::black)
  {
    board_half = upper_board_half;
    di_offset = 0;
    ad_offset = 7;
  }
  if ((threatening_pawn_squares_Q & board_half) & (_s.other_Pawns | _s.other_King_Queens_or_Bishops))
    return true;
  if ((threatening_knight_squares_Q & board_half) & _s.other_Knights)
    return true;

  uint64_t tmp_square = square << 1;
  for (uint8_t index = 3; (tmp_square & _s.Rooks) == 0L; tmp_square <<= 1, index--)
  {
    // Is square threatened on file?
    if (file[index] & _s.other_Queens_or_Rooks)
    {
      for (uint64_t sq = (_col_to_move == col::white) ? tmp_square >> 8 : tmp_square <<= 8;
          true;
          (_col_to_move == col::white) ? sq >>= 8 : sq <<= 8)
      {
        if (sq & _s.other_Queens_or_Rooks)
          return true;
        if (sq & (_s.all_pieces))
          break;
      }
    }
    // Is square threatened on diagonal?
    if (diagonal[index + di_offset] & _s.other_Queens_or_Bishops)
    {
      for (uint64_t sq = (_col_to_move == col::white) ? tmp_square >> 9 : tmp_square << 9;
          true;
          (_col_to_move == col::white) ? sq >>= 9 : sq <<= 9)
      {
        if (sq & _s.other_Queens_or_Bishops)
          return true;
        if (sq & _s.all_pieces)
          break;
      }
    }
    // Is square threatened on anti diagonal?
    if (anti_diagonal[index + ad_offset] & _s.other_Queens_or_Bishops)
    {
      for (uint64_t sq = (_col_to_move == col::white) ? tmp_square >> 7 : tmp_square << 7;
          true;
          (_col_to_move == col::white) ? sq >>= 7 : sq <<= 7)
      {
        if (sq & _s.other_Queens_or_Bishops)
          return true;
        if (sq & _s.all_pieces)
          break;
      }
    }
  }
  return false;
}

void Bitboard::find_short_castling(const uint64_t square)
{
  // Next if-statement should be true if the initial FEN-string's castling-rights wasn't bad.
  if (square == _s.King_initial_square && ((square >> 3) & _s.Rooks))
  {
    if (_s.King_rank & castling_empty_squares_K & _s.all_pieces)
      return;
    if (castling_squares_are_threatened_K(square))
      return;
    _movelist.push_front(BitMove(piecetype::King, move_props_castling, _s.King_initial_square, _s.King_initial_square >> 2));
  }
}

void Bitboard::find_long_castling(const uint64_t square)
{
  if (square == _s.King_initial_square && ((square << 4) & _s.Rooks))
  {
    if (_s.King_rank & castling_empty_squares_Q & _s.all_pieces)
      return;
    if (castling_squares_are_threatened_Q(square))
      return;
    _movelist.push_front(BitMove(piecetype::King, move_props_castling, _s.King_initial_square, _s.King_initial_square << 2));
  }
}

void Bitboard::find_normal_legal_moves()
{
  int8_t from_f, from_r;
  for (from_r = 1; from_r <= 8; from_r++)
  {
    if ((rank[from_r] & _s.own_pieces))
    {
      uint64_t square = file[a] & rank[from_r];
      for (from_f = a; from_f <= h; from_f++, square >>= 1)
      {
        //cout << "--------------------------" << endl << to_binary_board(square) << endl;
        if (square & (_s.own_pieces))
        {
          if (square & _s.pinned_pieces)
            find_legal_moves_for_pinned_piece(square);
          else if (square & _s.Pawns)
            find_Pawn_moves(square);
          else if (square & _s.Knights)
            find_Knight_moves(square);
          else if (square & _s.Bishops)
            find_Bishop_or_Queen_moves(square, piecetype::Bishop);
          else if (square & _s.Rooks)
            find_Rook_or_Queen_moves(square, piecetype::Rook);
          else if (square & _s.Queens)
          {
            find_Rook_or_Queen_moves(square, piecetype::Queen);
            find_Bishop_or_Queen_moves(square, piecetype::Queen);
          }
          else if (square & _s.King)
          {
            // Only castling remains to examine.
            find_short_castling(square);
            find_long_castling(square);
          }
        }
      }
    }
  }
}

// Sub-function to find_moves_to_square()
inline uint64_t Bitboard::step_out_from_square(uint64_t square,
                                               int8_t inc,
                                               piecetype& p_type,
                                               uint64_t& pieces)
{
  uint64_t allowed_squares = whole_board; // for inc 8 and -8 (files)
  uint64_t second = _s.Rooks; // for inc 8, -8, 1, -1 (files and ranks)
  switch (inc)
  {
    case -7:
      case 9:
      second = _s.Bishops;
      allowed_squares = not_a_file;
      break;
    case 1:
      allowed_squares = not_a_file;
      break;
    case 7:
      case -9:
      allowed_squares = not_h_file;
      second = _s.Bishops;
      break;
    case -1:
      allowed_squares = not_h_file;
      break;
  }
  for (uint64_t sq = (inc > 0) ? square >> inc : square << -inc;
      (sq & allowed_squares);
      (inc > 0) ? sq >>= inc : sq <<= -inc)
  {
    if (sq & _s.Queens)
    {
      p_type = piecetype::Queen;
      pieces = _s.Queens;
      return sq;
    }
    if (sq & second)
    {
      return sq;
    }
    if (sq & _s.all_pieces)
      return 0L;
  }
  return 0L;
}

// Sub-function to try_adding_ep_pawn_move()
bool Bitboard::check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square, uint8_t inc)
{
  uint64_t square;
  // Check if other_pawn_square is to the (east or north of the King)
  // or to the (west or south of the King), so we now which way to shift.
  // Step out from the King in the direction of other_pawn_square.

  // When file_condition becomes false we have reached the edge of the board
  // and moved to a square on the other side of the board, so the loop should stop.
  // This condition is valid here because we never start at the a_file or h_file,
  // pins on the king-file pose no problem.
  // If we instead go over the bottom or top edge of the board, square will be 0L
  // and the loop will stop.
  uint64_t file_condition = ((long int) (_s.King - _s.checking_piece_square) > 0) ? not_a_file : not_h_file;

  uint64_t pinning_pieces = (abs(inc) == 1 || abs(inc) == 8) ? _s.other_Queens_or_Rooks : _s.other_Queens_or_Bishops;
  // abs(inc) == 8 should never happen, but I keep it in case I want to use this approach somewhere else in the code.

  for (square = ((long int) (_s.King - _s.checking_piece_square) > 0) ? _s.King >> inc : _s.King << inc;
      (square & file_condition);
      ((long int) (_s.King - _s.checking_piece_square) > 0) ? square >>= inc : square <<= inc)
  {
    // Can we put any piece between our King and the checking piece?
    if (square & pinning_pieces)
      return true;
    // Continue loop if empty squares, other_pawn_square or own_pawn_square is found.
    // own_pawn_square can be a blocker on a rank together with other_pawn_square
    // which both would be gone from the rank after the move.
    if ((square & _s.empty_squares) || (square & other_pawn_square) || (square & own_pawn_square))
      continue;
    break; // We have found another blocking piece.
  }
  return false;
}

// Sub-function to step_from_King_to_checking_piece().
// preconditions:
// - to_square should be empty
void Bitboard::find_pawn_moves_to_empty_square(uint64_t to_square)
{
  uint64_t from_square;
  uint64_t not_pinned_pieces = whole_board ^ _s.pinned_pieces;

  from_square = (_col_to_move == col::white) ? to_square << 8 : to_square >> 8; // one step
  // No pawn moves possible if from_sqare doesn't contain a pawn of
  // color _col_to_move, which isn't pinned.
  if ((from_square & _s.Pawns) && (from_square & not_pinned_pieces))
  {
    add_pawn_move_check_promotion(from_square, to_square);
  }
  else if ((from_square & _s.empty_squares) && (to_square & ((_col_to_move == col::white) ? row_4 : row_5)))
  {
    // No blocking piece and correct row for a for two-squares-pawn-move
    (_col_to_move == col::white) ? from_square <<= 8 : from_square >>= 8; // two steps
    if ((from_square & _s.Pawns) && (from_square & not_pinned_pieces))
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

// Sub-function to step_from_King_to_checking_piece()
void Bitboard::find_moves_to_square(uint64_t to_square)
{
  piecetype p_type;
  uint64_t from_square;
  uint64_t not_pinned_pieces = whole_board ^ _s.pinned_pieces;

  if (to_square & _s.empty_squares)
  {
    find_pawn_moves_to_empty_square(to_square);
  }
  else if (to_square & _s.other_pieces)
  {
    // Pawn Captures (e.p. not possible)
    if (to_square & not_h_file)
    {
      from_square = (_col_to_move == col::white) ? to_square << 7 : to_square >> 9;
      if ((from_square & _s.Pawns) && (from_square & not_pinned_pieces))
        add_pawn_move_check_promotion(from_square, to_square);
    }
    if (to_square & not_a_file)
    {
      from_square = (_col_to_move == col::white) ? to_square << 9 : to_square >> 7;
      if ((from_square & _s.Pawns) && (from_square & not_pinned_pieces))
        add_pawn_move_check_promotion(from_square, to_square);
    }
  }

  find_Knight_moves_to_square(to_square);

  uint64_t pieces;
  // File
  if ((from_square = step_out_from_square(to_square,
                                          8,
                                          p_type = piecetype::Rook,
                                          pieces = _s.Rooks)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

  if ((from_square = step_out_from_square(to_square,
                                          -8,
                                          p_type = piecetype::Rook,
                                          pieces = _s.Rooks)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

  // Rank
  if ((from_square = step_out_from_square(to_square,
                                          1,
                                          p_type = piecetype::Rook,
                                          pieces = _s.Rooks)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

  if ((from_square = step_out_from_square(to_square,
                                          -1,
                                          p_type = piecetype::Rook,
                                          pieces = _s.Rooks)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

// Diagonal
  if ((from_square = step_out_from_square(to_square,
                                          9,
                                          p_type = piecetype::Bishop,
                                          pieces = _s.Bishops)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

  if ((from_square = step_out_from_square(to_square,
                                          -9,
                                          p_type = piecetype::Bishop,
                                          pieces = _s.Bishops)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

// Anti-diagonal
  if ((from_square = step_out_from_square(to_square,
                                          7,
                                          p_type = piecetype::Bishop,
                                          pieces = _s.Bishops)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);

  if ((from_square = step_out_from_square(to_square,
                                          -7,
                                          p_type = piecetype::Bishop,
                                          pieces = _s.Bishops)))
    try_adding_move(pieces, p_type, move_props_none, from_square, to_square);
}

// Sub-function to find_moves_after_check()
void Bitboard::step_from_King_to_checking_piece(uint8_t inc)
{
  uint64_t square;
  // Check if the checking_piece is to the (east or north of the King)
  // or to the (west or south of the King), so we now which way to shift.
  // Step out from the King in the direction of the checking piece.
  for (square = ((long int) (_s.King - _s.checking_piece_square) > 0) ? _s.King >> inc : _s.King << inc;
      (square & _s.checking_piece_square) == 0L;
      ((long int) (_s.King - _s.checking_piece_square) > 0) ? square >>= inc : square <<= inc)
  {
    // Can we put any piece between our King and the checking piece?
    find_moves_to_square(square);
  }
  // Loop stopped at the square of the checking piece.
  // Can we take it?
  find_moves_to_square(square);
  // Also check if the piece can be taken e.p.
  if (_ep_square & ((_col_to_move == col::white) ? square >> 8 : square << 8))
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

void Bitboard::find_moves_after_check()
{
  // All King-moves has already been found.
  if (_s.checking_piece_square & _s.King_file)
    step_from_King_to_checking_piece(8); // along the file
  else if (_s.checking_piece_square & _s.King_rank)
    step_from_King_to_checking_piece(1); // along the rank
  else if (_s.checking_piece_square & _s.King_diagonal)
    step_from_King_to_checking_piece(9); // along the diagonal
  else if (_s.checking_piece_square & _s.King_anti_diagonal)
    step_from_King_to_checking_piece(7); // along the anti-diagonal
}

void Bitboard::find_all_legal_moves()
{
  init_piece_state();
  find_king_moves();
  look_for_checks_and_pinned_pieces();
  if (_s.check_count > 1)
  {
    // It's a double-check. We're finished.
    // Only King-moves are possible and we've
    // already figured them out.
  }
  else if (_s.check_count == 1)
  {
    // If it's a single check, we must see if
    // the checking piece can be taken or if we
    // can put a piece in between that piece
    // and the King.
    find_moves_after_check();
    return;
  }
  else
  {
    // Not a check. We're free to use all our
    // pieces
    find_normal_legal_moves();
  }
}

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
  if (_W_Pawns & square)
  {

  }
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
      _B_Queens ^= square, _material_diff += 9, pt = piecetype::Queen;
    else if (_B_Pawns & square)
      _B_Pawns ^= square, _material_diff += 1, pt = piecetype::Pawn;
    else if (_B_Rooks & square)
      _B_Rooks ^= square, _material_diff += 5, pt = piecetype::Rook;
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
      _W_Rooks ^= square, _material_diff -= 5, pt = piecetype::Rook;
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

// TOD: continue work here:
inline void Bitboard::remove_castling_rights()
{
  uint8_t cr = (_col_to_move == col::white) ? castling_rights_W : castling_rights_B;
  if (_castling_rights & cr)
  {
    // Even if A & B is true the result can have different bits set.
    // So we still have to check if A & B & C is true.
    if (_castling_rights & cr & castling_rights_WK)
    {
      _hash_tag ^= hash_table._castling_rights[0];
    }
    if (_castling_rights & cr & castling_rights_WQ)
    {
      _hash_tag ^= hash_table._castling_rights[1];
    }
    if (_castling_rights & cr & castling_rights_BK)
    {
      _hash_tag ^= hash_table._castling_rights[2];
    }
    if (_castling_rights & cr & castling_rights_BQ)
    {
      _hash_tag ^= hash_table._castling_rights[3];
    }
  }
}

// Set a "unique" hash tag for the position after
// adding or removing one piece from a square.
inline void Bitboard::update_hash_tag(uint64_t square, col p_color, piecetype p_type)
{
  _hash_tag ^= hash_table._random_table[LOG2(square)][index(p_color)][index(p_type)];
}

// Set a "unique" hash tag for the position after
// removing one piece from a square and putting
// the same piece on an empty square.
inline void Bitboard::update_hash_tag(uint64_t square1, uint64_t square2, col p_color, piecetype p_type)
{
  _hash_tag ^= (hash_table._random_table[LOG2(square1)][index(p_color)][index(p_type)] ^
                hash_table._random_table[LOG2(square2)][index(p_color)][index(p_type)]);
}

// Move a piece
// Preconditions:
// - i < movelist.size()
// - the move must be valid
void Bitboard::make_move(int i)
{
  BitMove m = _movelist[i];
  uint64_t to_square = m.to_square;
  uint64_t from_square = m.from_square;

// Clear _ep_square but remember its value.
//uint64_t tmp_ep_square = _ep_square;
  _ep_square = 0;

// Remove possible piece of other color on to_square and
// Then make the move (updates hashtag)
  if (to_square & _s.other_pieces)
    remove_other_piece(to_square);
  move_piece(from_square, to_square, m.piece_type);

// OK we have moved the piece, now we must
// look at some special cases.
  if (m.piece_type == piecetype::King)
  {
    // Move the rook if it's actually a castling move.
    if (m.properties == move_props_castling)
    {
      if (static_cast<long int>(from_square - to_square) > 0)
      {
        // Castling king side. TODO: is this right ?
        if (_col_to_move == col::white)
        {
          _W_Rooks ^= (h1_square | f1_square);
          update_hash_tag(h1_square, f1_square, _col_to_move, piecetype::Rook);
        }
        else
        {
          _B_Rooks ^= (h8_square | f8_square);
          update_hash_tag(h8_square, f8_square, _col_to_move, piecetype::Rook);
        }
      }
      else
      {
        // Castling queen side. TODO: is this right ?
        if (_col_to_move == col::white)
        {
          _W_Rooks ^= (a1_square | d1_square);
          update_hash_tag(a1_square, d1_square, _col_to_move, piecetype::Rook);
        }
        else
        {
          _B_Rooks ^= (a8_square | d8_square);
          update_hash_tag(a8_square, d8_square, _col_to_move, piecetype::Rook);
        }
      }
    }
    if (from_square & e8_square)
      remove_castling_rights();
  }
  else if (m.piece_type == piecetype::Rook)
  {
    // Clear castling rights for one side if applicable.
    if (to_square & _W_Rooks)
    {
      if (from_square & a1_square)
        _castling_rights &= !castling_rights_WQ;
      if (from_square & h1_square)
        _castling_rights &= !castling_rights_WK;
    }
    else
    {
      // Black rook
      if (from_square & a8_square)
        _castling_rights &= !castling_rights_BQ;
      if (from_square & h8_square)
        _castling_rights &= !castling_rights_BK;
    }
  }
  else if (m.piece_type == piecetype::Pawn)
  {
    if (m.properties == move_props_en_passant)
    {
      // Remove the pawn taken e.p.
      (_col_to_move == col::white) ? _B_Pawns ^= _ep_square, _material_diff += 1.0 :
                                     _W_Pawns ^= _ep_square, _material_diff -= 1.0;
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
          _ep_square = to_square << 8;
      }
    }
    else
    {
      if ((from_square & row_7) && (to_square & row_5))
      {
        if (((to_square & not_a_file) && ((to_square << 1) & _s.other_Pawns)) ||
            ((to_square & not_h_file) && ((to_square >> 1) & _s.other_Pawns)))
          _ep_square = to_square >> 8;
      }
    }
  }
  else if (m.promotion_piece_type != piecetype::Undefined)
  {
// Remove the pawn from promotion square
// subtract 1 from the normal piece-values
// because the pawn disappears from the board.
    (_col_to_move == col::white) ? _W_Pawns ^= to_square : _B_Pawns ^= to_square;
    switch (m.promotion_piece_type)
    {
      case piecetype::Queen:
        (_col_to_move == col::white) ? _material_diff += 8.0, _W_Queens |= to_square :
                                       _material_diff -= 8.0, _B_Queens |= to_square;
        break;
      case piecetype::Rook:
        (_col_to_move == col::white) ? _material_diff += 4.0, _W_Rooks |= to_square :
                                       _material_diff -= 4.0, _B_Rooks |= to_square;
        break;
      case piecetype::Knight:
        (_col_to_move == col::white) ? _material_diff += 2.0, _W_Knights |= to_square :
                                       _material_diff -= 2.0, _B_Knights |= to_square;
        break;
      case piecetype::Bishop:
        (_col_to_move == col::white) ? _material_diff += 2.0, _W_Bishops |= to_square :
                                       _material_diff -= 2.0, _B_Bishops |= to_square;
        break;
      default:
        ;
    }
  }
// init _material_diff
  init_piece_state();
}

} // namespace C2_chess

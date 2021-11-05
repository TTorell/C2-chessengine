//============================================================================
// Name        : Bitboard.cpp
// Author      : Torsten
// Version     :
// Copyright   : Your copyright notice
// Description : C++ code for a bitboard-representation of a chess board.
//============================================================================

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

std::atomic<bool> Bitboard::time_left(false);
Bitboard Bitboard::level_boards[38];

Bitboard::Bitboard() :
    _hash_tag(zero),
    _movelist(),
    _col_to_move(col::white),
    _castling_rights(castling_rights_none),
    _ep_square(zero),
    _material_diff(0),
    _s(),
    _W_King(zero),
    _W_Queens(zero),
    _W_Rooks(zero),
    _W_Bishops(zero),
    _W_Knights(zero),
    _W_Pawns(zero),
    _B_King(zero),
    _B_Queens(zero),
    _B_Rooks(zero),
    _B_Bishops(zero),
    _B_Knights(zero),
    _B_Pawns(zero),
    _W_King_file(zero),
    _W_King_rank(zero),
    _B_King_file(zero),
    _B_King_rank(zero),
    _W_King_diagonal(zero),
    _W_King_anti_diagonal(zero),
    _B_King_diagonal(zero),
    _B_King_anti_diagonal(zero),
    _W_King_file_index(e),
    _W_King_rank_index(1),
    _B_King_file_index(e),
    _B_King_rank_index(8)
{
}

inline void Bitboard::clear_movelist()
{
  _movelist.clear();
}

// Inits the Bitboard position from a text string (Forsyth-Edwards Notation)
int Bitboard::read_position(const std::string& FEN_string)
{
  //std::cout << "\"" << FEN_string << "\"" << std::endl;
  _W_King = zero;
  _W_Queens = zero;
  _W_Rooks = zero;
  _W_Bishops = zero;
  _W_Knights = zero;
  _W_Pawns = zero;
  _B_King = zero;
  _B_Queens = zero;
  _B_Rooks = zero;
  _B_Bishops = zero;
  _B_Knights = zero;
  _B_Pawns = zero;

  std::vector<std::string> FEN_tokens = split(FEN_string, ' ');
  if (FEN_tokens.size() != 6)
  {
    std::cerr << "Read error: Number of elements in FEN string should be 6, but there are " << FEN_tokens.size() << std::endl;
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }

  int ri = 8;
  int fi = a;
  //char ch = '0';
  //unsigned int i;
  //for (i = 0; i < FEN_string.size() && ch != ' '; i++)
  for (const char& ch : FEN_tokens[0])
  {
    switch (ch)
    {
      case ' ':
        break;
      case 'K':
        _W_King_file_index = fi;
        _W_King_rank_index = ri;
        _W_King_file = file[fi];
        _W_King_rank = rank[ri];
        _W_King = _W_King_file & _W_King_rank;
        _W_King_diagonal = diagonal[8 - ri + fi];
        _W_King_anti_diagonal = anti_diagonal[fi + ri - 1];
        break;
      case 'k':
        _B_King_file_index = fi;
        _B_King_rank_index = ri;
        _B_King_file = file[fi];
        _B_King_rank = rank[ri];
        _B_King = _B_King_file & _B_King_rank;
        _B_King_diagonal = diagonal[8 - ri + fi];
        _B_King_anti_diagonal = anti_diagonal[fi + ri - 1];
        break;
      case 'Q':
        _W_Queens |= (file[fi] & rank[ri]), _material_diff += 9;
        break;
      case 'q':
        _B_Queens |= (file[fi] & rank[ri]), _material_diff -= 9;
        break;
      case 'B':
        _W_Bishops |= (file[fi] & rank[ri]), _material_diff += 3;
        break;
      case 'b':
        _B_Bishops |= (file[fi] & rank[ri]), _material_diff -= 3;
        break;
      case 'N':
        _W_Knights |= (file[fi] & rank[ri]), _material_diff += 3;
        break;
      case 'n':
        _B_Knights |= (file[fi] & rank[ri]), _material_diff -= 3;
        break;
      case 'R':
        _W_Rooks |= (file[fi] & rank[ri]), _material_diff += 5;
        break;
      case 'r':
        _B_Rooks |= (file[fi] & rank[ri]), _material_diff -= 5;
        break;
      case 'P':
        _W_Pawns |= (file[fi] & rank[ri]), _material_diff += 1;
        break;
      case 'p':
        _B_Pawns |= (file[fi] & rank[ri]), _material_diff -= 1;
        break;
      case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        fi += ch - '1';
        break;
      case '/':
        fi = a;
        ri--;
        if (ri < 1)
        {
          std::cerr << "Read error: corrupt FEN input (rank)" << "\n";
          std::cerr << "FEN-string: " << FEN_string << std::endl;
          return -1;
        }
        continue; // without increasing f
      default:
        std::cerr << "Read error: unexpected character in FEN input" << ch << "\n";
        std::cerr << "FEN-string: " << FEN_string << std::endl;
        return -1;
    } // end of case-statement
    if (fi > h)
    {
      std::cerr << "Read error: corrupt FEN input (file)" << "\n";
      std::cerr << "FEN-string: " << FEN_string << std::endl;
      return -1;
    }
    fi++;
  } // end of for-loop
  _col_to_move = col::white;
  if (FEN_tokens[1] == "b")
    update_col_to_move();
  std::string castling_rights = FEN_tokens[2];
  _castling_rights = castling_rights_none;
  for (const char& cr : castling_rights)
  {
    switch (cr)
    {
      case '-':
        _castling_rights = castling_rights_none;
        break;
      case 'K':
        _castling_rights |= castling_right_WK;
        break;
      case 'Q':
        _castling_rights |= castling_right_WQ;
        break;
      case 'k':
        _castling_rights |= castling_right_BK;
        break;
      case 'q':
        _castling_rights |= castling_right_BQ;
        break;
      default:
        std::cerr << "Read error: Strange castling character \'" << cr << "\' in FEN-string." << std::endl;
        std::cerr << "FEN-string: " << FEN_string << std::endl;
        return -1;
    }
  }
  std::string ep_string = FEN_tokens[3];
  if (!regexp_match(ep_string, "^-|([a-h][36])$")) // Either a "-" or a square on rank 3 or 6-
  {
    std::cerr << "Read error: Strange en_passant_square characters \"" << ep_string << "\" in FEN-string." << std::endl;
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }
  _ep_square = zero;
  if (ep_string.length() == 2)
  {
    _ep_square = file[ep_string[0] - 'a'] & rank[ep_string[1] - '0'];
  }
  //std::cout << "ep_square: " << std::endl << to_binary_board(_ep_square) << std::endl;

  return 0;
}

void Bitboard::init_piece_state()
{
  clear_movelist();
  _s.pinned_pieces = zero;
  _s.checkers = zero;

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
    _s.castling_rights_K = _castling_rights & castling_right_WK;
    _s.castling_rights_Q = _castling_rights & castling_right_WQ;
    _s.own_pieces = _W_King | _W_Queens | _W_Rooks | _W_Bishops | _W_Knights | _W_Pawns;
    _s.adjacent_files = (_W_King_file_index - 1 >= a) ? file[_W_King_file_index - 1] : zero;
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

    _s.other_King = _B_King;
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
    _s.King_initial_square = e8_square;
    _s.castling_rights_K = _castling_rights & castling_right_BK;
    _s.castling_rights_Q = _castling_rights & castling_right_BQ;
    _s.own_pieces = _B_King | _B_Queens | _B_Rooks | _B_Bishops | _B_Knights | _B_Pawns;
    _s.adjacent_files = (_B_King_file_index - 1 >= a) ? file[_B_King_file_index - 1] : zero;
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

    _s.other_King = _W_King;
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

bool Bitboard::square_is_threatened(uint64_t to_square, bool King_is_asking)
{
  uint64_t possible_attackers, attacker;
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
  if (_s.other_Queens_or_Rooks)
  {
    // Check threats on file and rank
    uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
    possible_attackers = to_ortogonal_squares & _s.other_Queens_or_Rooks;
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_ortogonal_squares) & tmp_all_pieces) == zero)
        return true;
    }
  }

  // Check diagonal threats
  if (_s.other_Queens_or_Bishops)
  {
    uint64_t to_diagonal_squares = diagonal_squares(to_square);
    possible_attackers = to_diagonal_squares & _s.other_Queens_or_Bishops;
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
  king_moves &= (~_s.own_pieces);
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
    pinned_piece = zero;
    return true;
  }
  if (square & threatening_pieces)
  {
    if (pinned_piece == zero)
    {
      _s.checkers |= square;
    }
    _s.pinned_pieces |= pinned_piece;
    return true;
  }
  if (square & _s.own_pieces)
  {
    if (pinned_piece) // Two of King's own pieces are in between.
    {
      pinned_piece = zero;
      return true;
    }
    else
      pinned_piece |= (square); // Not sure if it's pinned yet though
  }
  return false;
}

inline void Bitboard::contains_checking_piece(const uint64_t square,
                                              const uint64_t pieces,
                                              const uint64_t forbidden_files = zero)
{
  if ((_s.King & forbidden_files) == zero)
  {
    if (square & pieces)
    {
      _s.checkers |= square;
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
  uint64_t possible_checkers, possible_checker, between_squares;
  uint8_t King_file_idx = file_idx(_s.King);

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
  if (_s.other_Queens_or_Rooks)
  {
    uint64_t King_ortogonal_squares = ortogonal_squares(_s.King);
    possible_checkers = King_ortogonal_squares & _s.other_Queens_or_Rooks;
    while (possible_checkers)
    {
      possible_checker = popright_square(possible_checkers);
      between_squares = between(_s.King, possible_checker, King_ortogonal_squares);
      if ((between_squares & _s.all_pieces) == zero)
        _s.checkers |= possible_checker;
      else if (std::popcount(between_squares & _s.all_pieces) == 1)
        _s.pinned_pieces |= (between_squares & _s.own_pieces);
    }
  }

  // Check diagonal threats
  if (_s.other_Queens_or_Bishops)
  {
    uint64_t King_diagonal_squares = diagonal_squares(_s.King);
    possible_checkers = King_diagonal_squares & _s.other_Queens_or_Bishops;
    while (possible_checkers)
    {
      possible_checker = popright_square(possible_checkers);
      between_squares = between(_s.King, possible_checker, King_diagonal_squares, true); // true = diagonal
      if ((between_squares & _s.all_pieces) == zero)
        _s.checkers |= possible_checker;
      else if (std::popcount(between_squares & _s.all_pieces) == 1)
        _s.pinned_pieces |= (between_squares & _s.own_pieces);
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
      (to_square & pinning_pieces) == zero;
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
    else if (from_square & (_s.Queens))
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
    to_square = (_col_to_move == col::white) ? from_square >> 9 : from_square << 9;
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
    else if (from_square & _s.Queens)
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
    to_square = (_col_to_move == col::white) ? from_square >> 7 : from_square << 7;
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
    else if (from_square & _s.Queens)
      step_from_King_to_pinning_piece(from_square, 7, piecetype::Queen, _s.other_Queens_or_Bishops);
    else if (from_square & (_s.Bishops))
      step_from_King_to_pinning_piece(from_square, 7, piecetype::Bishop, _s.other_Queens_or_Bishops);
    return;
  }
}

// Inserts move based on the content of to_square and
// if the piece is pinned or not.
// A Move to a square occupied by own piece is ignored.
// If to_square == zero ("above" or "below" the board)
// no move will be added,
inline void Bitboard::try_adding_move(uint64_t pieces,
                                      piecetype p_type,
                                      uint8_t move_props,
                                      uint64_t from_square,
                                      uint64_t to_square)
{
  if (from_square & pieces & (~_s.pinned_pieces))
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
  assert(std::has_single_bit(from_square));
  if (from_square & (~_s.pinned_pieces))
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
  else
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

void Bitboard::find_Pawn_moves()
{
  uint64_t to_square, moved_pawns;
  uint64_t pawns = _s.Pawns & ~(_s.pinned_pieces);
  // Shift unpinned pawns one square ahead. Check if the new square is empty:
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
  pawns = _s.Pawns & (~_s.pinned_pieces);
  moved_pawns = ((_col_to_move == col::white) ? pawns >> 9 : pawns << 7) & not_a_file;
  while (moved_pawns)
  {
    to_square = popright_square(moved_pawns);
    if (to_square & _s.other_pieces)
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 9 :
                                                                   to_square >> 7,
                                    to_square);
    if (to_square == _ep_square)
      try_adding_ep_pawn_move((_col_to_move == col::white) ? to_square << 9 : to_square >> 7);
  }
  moved_pawns = ((_col_to_move == col::white) ? pawns >> 7 : pawns << 9) & not_h_file;
  while (moved_pawns)
  {
    to_square = popright_square(moved_pawns);
    if (to_square & _s.other_pieces)
      add_pawn_move_check_promotion((_col_to_move == col::white) ? to_square << 7 :
                                                                   to_square >> 9,
                                    to_square);
    if (to_square == _ep_square)
      try_adding_ep_pawn_move((_col_to_move == col::white) ? to_square << 7 : to_square >> 9);
  }
}

void Bitboard::find_Knight_moves()
{
  uint64_t from_square, to_square, to_squares;
  if (_s.Knights)
  {
    uint64_t knights = _s.Knights & (~_s.pinned_pieces);
    while (knights)
    {
      from_square = popright_square(knights);
      to_squares = adjust_pattern(knight_pattern, from_square) & (~_s.own_pieces);
      while (to_squares)
      {
        to_square = popright_square(to_squares);
        if (to_square & _s.other_pieces)
          _movelist.push_back(BitMove(piecetype::Knight, move_props_capture, from_square, to_square));
        else
          _movelist.push_front(BitMove(piecetype::Knight, move_props_none, from_square, to_square));
      }
    }
  }
}

void Bitboard::find_Knight_moves_to_square(const uint64_t to_square)
{
  assert(to_square & (~_s.own_pieces));
  if (_s.Knights)
  {
    uint64_t knights = adjust_pattern(knight_pattern, to_square) & _s.Knights;
    while (knights)
    {
      uint64_t knight = popright_square(knights);
      if (knight & (~_s.pinned_pieces))
      {
        if (to_square & _s.other_pieces)
          _movelist.push_back(BitMove(piecetype::Knight, move_props_capture, knight, to_square));
        else
          _movelist.push_front(BitMove(piecetype::Knight, move_props_none, knight, to_square));
      }
    }
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

void Bitboard::find_short_castling()
{
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
    _movelist.push_front(BitMove(piecetype::King, move_props_castling, _s.King_initial_square, _s.King_initial_square >> 2));
  }
}

void Bitboard::find_long_castling()
{
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
    _movelist.push_front(BitMove(piecetype::King, move_props_castling, _s.King_initial_square, _s.King_initial_square << 2));
  }
}

void Bitboard::find_legal_moves_for_pinned_pieces()
{
  uint64_t squares = _s.pinned_pieces;
  while (squares)
    find_legal_moves_for_pinned_piece(popright_square(squares));
}

void Bitboard::find_normal_legal_moves()
{
  find_Pawn_moves();
  find_Knight_moves();

  uint64_t pieces = _s.own_pieces;
  while (pieces)
  {
    uint64_t square = popright_square(pieces);
    if (square & _s.pinned_pieces)
      find_legal_moves_for_pinned_piece(square);
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
      if ((_col_to_move == col::white && square == e1_square) ||
          (_col_to_move == col::black && square == e8_square))
      {
        find_short_castling();
        find_long_castling();
      }
    }
  }
}

bool Bitboard::check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square)
{
  uint64_t possible_pinners, possible_pinner, between_squares;
// Check file and rank
  if (_s.other_Queens_or_Rooks)
  {
    uint64_t King_ortogonal_squares = ortogonal_squares(_s.King);
    if (King_ortogonal_squares & other_pawn_square)
    {
      possible_pinners = King_ortogonal_squares & _s.other_Queens_or_Rooks;
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
  if (_s.other_Queens_or_Bishops)
  {
    uint64_t King_diagonal_squares = diagonal_squares(_s.King);
    if (King_diagonal_squares & other_pawn_square)
    {
      possible_pinners = King_diagonal_squares & _s.other_Queens_or_Bishops;
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

// The following method doesn't consider possible King-moves to square
void Bitboard::find_moves_to_square(uint64_t to_square)
{
  assert((to_square & (~_s.own_pieces)) && std::has_single_bit(to_square));
  uint64_t from_square;
  uint64_t move_candidates, move_candidate;
  uint64_t between_squares;
  uint64_t not_pinned_pieces = ~_s.pinned_pieces;
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

// Check file and rank
  uint64_t Queens_or_Rooks = _s.Queens | _s.Rooks;
  if (Queens_or_Rooks)
  {
    uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
    move_candidates = to_ortogonal_squares & Queens_or_Rooks;
    while (move_candidates)
    {
      move_candidate = popright_square(move_candidates);
      if (move_candidate & not_pinned_pieces)
      {
        between_squares = between(to_square, move_candidate, to_ortogonal_squares);
        if ((between_squares & _s.all_pieces) == zero)
        {
          piecetype p_type = (to_square & _s.Queens) ? piecetype::Queen : piecetype::Rook;
          if (to_square & (~_s.all_pieces))
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
      if (move_candidate & not_pinned_pieces)
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
  uint64_t between_squares, between_square;
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

} // namespace C2_chess


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
Zobrist_bitboard_hash Bitboard::bb_hash_table;

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

inline void Bitboard::init_piece_state()
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

bool Bitboard::square_is_threatened(uint64_t square, bool King_is_asking)
{
  uint64_t possible_attackers, attacker;
  uint64_t tmp_all_pieces = _s.all_pieces;
  uint8_t f_idx = file_idx(square);

  // Check Pawn-threats
  if (_s.other_Pawns)
  {
    if ((f_idx != h) && (_s.other_Pawns & ((_col_to_move == col::white) ? square >> 9 : square << 7)))
      return true;
    if ((f_idx != a) && (_s.other_Pawns & ((_col_to_move == col::white) ? square >> 7 : square << 9)))
      return true;
  }

  // Check Knight-threats
  if (_s.other_Knights && ((adjust_pattern(knight_pattern, square) & _s.other_Knights)))
    return true;

  // Check King (and adjacent Queen-threats)
  if (adjust_pattern(king_pattern, square) & (_s.other_King | _s.other_Queens))
    return true;

  if (King_is_asking)
  {
    // Treat King-square as empty to catch xray-attacks through the King-square
    tmp_all_pieces ^= _s.King;
  }

  uint8_t r_idx = rank_idx(square);
  // Check threats on file and rank
  if (_s.other_Queens_or_Rooks)
  {
    // Check threats on file and rank
    uint64_t ortogonal_squares = file[f_idx] | rank[r_idx];
    possible_attackers = ortogonal_squares & _s.other_Queens_or_Rooks;
    while (possible_attackers)
    {
      attacker = to_square(popright_bit_idx(possible_attackers));
      if ((between(square, attacker, ortogonal_squares) & tmp_all_pieces) == zero)
        return true;
    }
  }

  // Check diagonal threats
  if (_s.other_Queens_or_Bishops)
  {
    uint64_t diagonal_squares = to_diagonal(f_idx, r_idx) | to_anti_diagonal(f_idx, r_idx);
    possible_attackers = diagonal_squares & _s.other_Queens_or_Bishops;
    while (possible_attackers)
    {
      attacker = to_square(popright_bit_idx(possible_attackers));
      if ((between(square, attacker, diagonal_squares, true) & tmp_all_pieces) == zero)
        return true;
    }
  }
  return false;
}

bool Bitboard::square_is_threatened_old(int8_t file_index, int8_t rank_index, bool King_is_asking)
{
  // -----------------------------------------------------------
  // Checks if square is threatened by any piece
  // of color "threatened_by_color".
  // -----------------------------------------------------------

  uint64_t square_file = file[file_index];
  uint64_t square_rank = rank[rank_index];
  uint64_t square = square_file & square_rank;

  uint64_t adjacent_files = file[file_index + 1];
  adjacent_files |= ((file_index - 1 >= a) ? file[file_index - 1] : zero);
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
  knight_ranks |= (rank_index - 2 >= 1) ? rank[rank_index - 2] : zero;
  if (adjacent_files & knight_ranks & _s.other_Knights)
    return true;
  // Check Knights two files from square
  uint64_t knight_files = file[file_index + 2];
  knight_files |= (file_index - 2 >= a) ? file[file_index - 2] : zero;
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
  int8_t fi, ri;
  if ((square_file ^ square) & _s.other_Queens_or_Rooks)
  {
    // Opponent has at least one Queen or Rook on the same file
    // Step downwards from the square.
    for (ri = rank_index - 1; ri >= 1; ri--)
    {
      if (square_file & rank[ri] & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (square_file & rank[ri] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // Step upwards from the square.
    for (ri = rank_index + 1; ri <= 8; ri++)
    {
      if (square_file & rank[ri] & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (square_file & rank[ri] & (other_pieces | own_pieces))
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
    for (fi = file_index - 1; fi >= a; fi--)
    {
      if (file[fi] & square_rank & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (file[fi] & square_rank & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // We haven't found the queen_or rook yet.
    // Step to the right from the square.
    for (fi = file_index + 1; fi <= h; fi++)
    {
      if (file[fi] & square_rank & _s.other_Queens_or_Rooks)
      {
        return true;
      }
      if (file[fi] & square_rank & (other_pieces | own_pieces))
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
    for (fi = file_index + 1, ri = rank_index + 1; fi <= h && ri <= 8; fi++, ri++)
    {
      if (file[fi] & rank[ri] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[fi] & rank[ri] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // Step south-west from the square.
    for (fi = file_index - 1, ri = rank_index - 1; fi >= a && ri >= 1; fi--, ri--)
    {
      if (file[fi] & rank[ri] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[fi] & rank[ri] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
  }
  if ((anti_diagonal[file_index + rank_index - 1] ^ square) & _s.other_Queens_or_Bishops)
  {
    // Step north-west from the square.
    for (fi = file_index - 1, ri = rank_index + 1; fi >= a && ri <= 8; fi--, ri++)
    {
      if (file[fi] & rank[ri] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[fi] & rank[ri] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
    }
    // Step south-east from the square.
    for (fi = file_index + 1, ri = rank_index - 1; fi <= h && ri >= 1; fi++, ri--)
    {
      if (file[fi] & rank[ri] & _s.other_Queens_or_Bishops)
      {
        return true;
      }
      if (file[fi] & rank[ri] & (other_pieces | own_pieces))
      {
        // Other piece found, no threat possible.
        break;
      }
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
    uint8_t square_idx = popright_bit_idx(king_moves);
    uint64_t square = to_square(square_idx);
//    uint8_t fi = file_idx(square);
//    uint8_t ri = rank_idx(square);
    if (!square_is_threatened(square, true))
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

void Bitboard::find_checkers_and_pinned_pieces()
{
  uint64_t possible_checkers, possible_checker, between_squares;
  uint8_t King_file_idx = file_idx(_s.King);
  uint8_t King_rank_idx = rank_idx(_s.King);

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
    uint64_t King_ortogonal_squares = (file[King_file_idx] | rank[King_rank_idx]);
    possible_checkers = King_ortogonal_squares & _s.other_Queens_or_Rooks;
    while (possible_checkers)
    {
      possible_checker = to_square(popright_bit_idx(possible_checkers));
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
    uint64_t King_diagonal_squares = (to_diagonal(King_file_idx, King_rank_idx) |
                                      to_anti_diagonal(King_file_idx, King_rank_idx));
    possible_checkers = King_diagonal_squares & _s.other_Queens_or_Bishops;
    while (possible_checkers)
    {
      possible_checker = to_square(popright_bit_idx(possible_checkers));
      between_squares = between(_s.King, possible_checker, King_diagonal_squares, true); // true = diagonal
      if ((between_squares & _s.all_pieces) == zero)
        _s.checkers |= possible_checker;
      else if (std::popcount(between_squares & _s.all_pieces) == 1)
        _s.pinned_pieces |= (between_squares & _s.own_pieces);
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
  _s.pinned_pieces = zero;
  uint64_t pinned_piece = zero;
// Check Queen or Rook on same file or rank
  int8_t fi, ri;
  uint64_t square;
  uint64_t other_pieces = _s.other_pieces ^ _s.other_Queens_or_Rooks;
  if (_s.King_file & _s.other_Queens_or_Rooks)
  {
    // Opponent has a Queen or Rook on the same file
    // Step downwards from the square.
    for (ri = _s.King_rank_index - 1; ri >= 1; ri--)
    {
      square = _s.King_file & rank[ri];
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Rooks,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step upwards from the square.
    pinned_piece = zero;
    for (ri = _s.King_rank_index + 1; ri <= 8; ri++)
    {
      square = _s.King_file & rank[ri];
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
    pinned_piece = zero;
    for (fi = _s.King_file_index - 1; fi >= a; fi--)
    {
      square = file[fi] & _s.King_rank;
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Rooks,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step east from square
    pinned_piece = zero;
    for (fi = _s.King_file_index + 1; fi <= h; fi++)
    {
      square = file[fi] & _s.King_rank;
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
    pinned_piece = zero;
    for (fi = _s.King_file_index + 1, ri = _s.King_rank_index + 1; fi <= h && ri <= 8; fi++, ri++)
    {
      square = file[fi] & rank[ri];
      // TODO if (square & _All_pieces)
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Bishops,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step south-west from the square.
    pinned_piece = zero;
    for (fi = _s.King_file_index - 1, ri = _s.King_rank_index - 1; fi >= a && ri >= 1; fi--, ri--)
    {
      square = file[fi] & rank[ri];
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
    pinned_piece = zero;
    for (fi = _s.King_file_index - 1, ri = _s.King_rank_index + 1; fi >= a && ri <= 8; fi--, ri++)
    {
      square = file[fi] & rank[ri];
      // TODO if (square & _All_pieces)
      if (find_check_or_pinned_piece(square,
                                     _s.other_Queens_or_Bishops,
                                     other_pieces,
                                     pinned_piece))
        break;
    }
    // Step south-east from the square.
    pinned_piece = zero;
    for (fi = _s.King_file_index + 1, ri = _s.King_rank_index - 1; fi <= h && ri >= 1; fi++, ri--)
    {
      square = file[fi] & rank[ri];
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

void Bitboard::find_Knight_moves_to_square(const uint64_t square)
{
  assert(square & (~_s.own_pieces));
  if (_s.Knights)
  {
    uint64_t knights = adjust_pattern(knight_pattern, square) & _s.Knights;
    while (knights)
    {
      uint64_t knight = to_square(popright_bit_idx(knights));
      if (knight & (~_s.pinned_pieces))
      {
        if (square & _s.other_pieces)
          _movelist.push_back(BitMove(piecetype::Knight, move_props_capture, knight, square));
        else
          _movelist.push_front(BitMove(piecetype::Knight, move_props_none, knight, square));
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
  for (uint8_t index = f; index <= g; index++, tmp_square >>= 1)
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
  for (uint8_t index = d; index >= b; tmp_square <<= 1, index--)
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
  if (_castling_rights & ((_col_to_move == col::white) ? castling_right_WK : castling_right_BK))
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
}

void Bitboard::find_long_castling(const uint64_t square)
{
  if (_castling_rights & ((_col_to_move == col::white) ? castling_right_WQ : castling_right_BQ))
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
            if ((_col_to_move == col::white && square == e1_square) ||
                (_col_to_move == col::black && square == e8_square))
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
      return zero;
  }
  return zero;
}

// Sub-function to try_adding_ep_pawn_move()
bool Bitboard::check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square, uint8_t inc)
{
  uint64_t square, file_condition, pinning_pieces;
// Check if other_pawn_square is (directly to the east or to the north of the King)
// or (directly to the west or to the south of the King), so we know which way to shift.
// Step out from the King in the direction of other_pawn_square.

// TODO: Can this method fail when the king is on a8, the square with the largest value.
// MSB is set (but only MSB is set). I cast the (unsigned long)-diff to a (long int).
// I don't think it can fail unless the other square is zero, but then something really
// wrong has happened before this.

// When file_condition becomes false we have reached the edge of the board
// and moved to a square on the other side of the board, so the loop should stop.
// This condition is valid here because we never start at the a_file or h_file,
// pins on the king-file pose no problem. So, inc == 8 is disregarded.
// If we instead go over the bottom or top edge of the board, square will be zero
// and the loop will stop.
  if (inc == 1)
  {
    // Along the rank
    file_condition = (static_cast<long int>(_s.King - other_pawn_square) > 0) ? not_a_file : not_h_file;
    pinning_pieces = _s.other_Queens_or_Rooks;
  }
  else if (inc == 9)
  {
    // Along the King-diagonal (moving up will also mean moving to the right).
    file_condition = (static_cast<long int>(_s.King - other_pawn_square) > 0) ? not_a_file : not_h_file;
    pinning_pieces = _s.other_Queens_or_Bishops;
  }
  else if (inc == 7)
  {
    // Along the King-anti-diagonal (moving up will also mean moving to the left).
    file_condition = (static_cast<long int>(_s.King - other_pawn_square) > 0) ? not_h_file : not_a_file;
    pinning_pieces = _s.other_Queens_or_Bishops;
  }
  else
    return false;

  for (square = (static_cast<long int>(_s.King - other_pawn_square) > 0) ? _s.King >> inc : _s.King << inc;
      (square & file_condition);
      (static_cast<long int>(_s.King - other_pawn_square) > 0) ? square >>= inc : square <<= inc)
  {
    if (square & pinning_pieces)
      return true; // We have reached the pinning piece without any interruption of the loop.
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

// The following method doesn't consider possible King-moves to square
void Bitboard::find_moves_to_square(uint64_t square)
{
  assert((square & (~_s.own_pieces)) && std::has_single_bit(square));
  uint64_t from_square;
  uint64_t move_candidates, move_candidate;
  uint64_t between_squares;
  uint64_t not_pinned_pieces = ~_s.pinned_pieces;
  if (square & _s.empty_squares)
  {
    find_pawn_moves_to_empty_square(square);
  }
  else if (square & _s.other_pieces)
  {
    // Pawn Captures (e.p. not possible)
    if (square & not_h_file)
    {
      from_square = (_col_to_move == col::white) ? square << 7 : square >> 9;
      if ((from_square & _s.Pawns) && (from_square & not_pinned_pieces))
        add_pawn_move_check_promotion(from_square, square);
    }
    if (square & not_a_file)
    {
      from_square = (_col_to_move == col::white) ? square << 9 : square >> 7;
      if ((from_square & _s.Pawns) && (from_square & not_pinned_pieces))
        add_pawn_move_check_promotion(from_square, square);
    }
  }

  find_Knight_moves_to_square(square);

  // Check file and rank
  uint8_t to_file_idx = file_idx(square);
  uint8_t to_rank_idx = rank_idx(square);
  uint64_t Queens_or_Rooks = _s.Queens | _s.Rooks;
  if (Queens_or_Rooks)
  {
    uint64_t ortogonal_squares = file[to_file_idx] | rank[to_rank_idx];
    move_candidates = ortogonal_squares & Queens_or_Rooks;
    while (move_candidates)
    {
      move_candidate = to_square(popright_bit_idx(move_candidates));
      if (move_candidate & not_pinned_pieces)
      {
        between_squares = between(square, move_candidate, ortogonal_squares);
        if ((between_squares & _s.all_pieces) == zero)
        {
          piecetype p_type = (square & _s.Queens) ? piecetype::Queen : piecetype::Rook;
          if (square & (~_s.all_pieces))
            _movelist.push_back(BitMove(p_type, move_props_none, move_candidate, square));
          else if (square & _s.other_pieces)
            _movelist.push_front(BitMove(p_type, move_props_capture, move_candidate, square));
        }
      }
    }
  }

  // Check diagonals
  uint64_t Queens_or_Bishops = _s.Queens | _s.Bishops;
  if (Queens_or_Bishops)
  {
    uint64_t diagonal_squares = (to_diagonal(to_file_idx, to_rank_idx) |
                                 to_anti_diagonal(to_file_idx, to_rank_idx));
    move_candidates = diagonal_squares & Queens_or_Bishops;
    while (move_candidates)
    {
      move_candidate = to_square(popright_bit_idx(move_candidates));
      if (move_candidate & not_pinned_pieces)
      {
        between_squares = between(square, move_candidate, diagonal_squares, true); // true = diagonal
        if ((between_squares & _s.all_pieces) == zero)
        {
          piecetype p_type = (square & _s.Queens) ? piecetype::Queen : piecetype::Bishop;
          if (square & ~_s.all_pieces)
            _movelist.push_back(BitMove(p_type, move_props_none, move_candidate, square));
          else if (square & _s.other_pieces)
            _movelist.push_front(BitMove(p_type, move_props_capture, move_candidate, square));
        }
      }
    }
  }

}

// Sub-function to find_moves_after_check()
void Bitboard::step_from_King_to_checking_piece(uint8_t inc)
{
  uint64_t square;
// Check if the checking_piece is to the (east or north of the King)
// or to the (west or south of the King), so we now which way to shift.
// Step out from the King in the direction of the checking piece.
  for (square = ((long int) (_s.King - _s.checkers) > 0) ? _s.King >> inc : _s.King << inc;
      (square & _s.checkers) == 0;
      ((long int) (_s.King - _s.checkers) > 0) ? square >>= inc : square <<= inc)
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

void Bitboard::find_moves_after_check(uint64_t checker)
{
// All possible King-moves have already been found.
  uint64_t between_squares, between_square;
  uint8_t King_file_idx = file_idx(_s.King);
  uint8_t King_rank_idx = rank_idx(_s.King);
  uint64_t King_ortogonal_squares = file[King_file_idx] | rank[King_rank_idx];
  if (checker & King_ortogonal_squares)
  {
    // Can we put any piece between the King and the checker?
    between_squares = between(_s.King, checker, King_ortogonal_squares);
    while (between_squares)
    {
      between_square = to_square(popright_bit_idx(between_squares));
      find_moves_to_square(between_square);
    }
  }
  else
  {
    uint64_t King_diagonal_squares = (to_diagonal(King_file_idx, King_rank_idx) |
                                      to_anti_diagonal(King_file_idx, King_rank_idx));
    if (checker & King_diagonal_squares)
    {
      // Can we put any piece between the King and the checker?
      between_squares = between(_s.King, checker, King_diagonal_squares, true);
      while (between_squares)
      {
        between_square = to_square(popright_bit_idx(between_squares));
        find_moves_to_square(between_square);
      }
    }
  }
  // Can we take the checking piece?
  find_moves_to_square(checker);

  // Also check if the checker is Pawn which can be taken e.p.
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

void Bitboard::find_moves_after_check_old()
{
// We never should call this method if it,s a double-check.
  assert(std::has_single_bit(_s.checkers));
// All King-moves has already been found.
  if (_s.checkers & _s.King_file)
    step_from_King_to_checking_piece(8); // along the file
  else if (_s.checkers & _s.King_rank)
    step_from_King_to_checking_piece(1); // along the rank
  else if (_s.checkers & _s.King_diagonal)
    step_from_King_to_checking_piece(9); // along the diagonal
  else if (_s.checkers & _s.King_anti_diagonal)
    step_from_King_to_checking_piece(7); // along the anti-diagonal
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

inline void Bitboard::update_col_to_move()
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


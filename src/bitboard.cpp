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

// Initializes the Bitboard position from a text string (Forsyth-Edwards Notation)
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
    std::cerr << "Read error: Number of tokens in FEN string should be 6, but there are " << FEN_tokens.size() << std::endl;
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }

  int ri = 8;
  int fi = a;
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
  return 0;
}

void Bitboard::init_piece_state()
{
  clear_movelist();
  _s.checkers = zero;
  _s.pinners = zero;
  _s.pinned_pieces = zero;

  if (_col_to_move == col::white)
  {
    _s.King = _W_King;
    _s.King_file_index = _W_King_file_index;
    _s.King_rank_index = _W_King_rank_index;
    _s.King_file = _W_King_file;
    _s.King_rank = _W_King_rank;
    _s.King_diagonal = _W_King_diagonal;
    _s.King_anti_diagonal = _W_King_anti_diagonal;
    _s.Queens = _W_Queens;
    _s.Rooks = _W_Rooks;
    _s.Bishops = _W_Bishops;
    _s.Knights = _W_Knights;
    _s.Pawns = _W_Pawns;
    _s.own_pieces = _s.King | _s.Queens | _s.Rooks | _s.Bishops | _s.Knights | _s.Pawns;

    _s.other_King = _B_King;
    _s.other_Queens = _B_Queens;
    _s.other_Rooks = _B_Rooks;
    _s.other_Bishops = _B_Bishops;
    _s.other_Knights = _B_Knights;
    _s.other_Pawns = _B_Pawns;
    _s.other_pieces = _s.other_King | _s.other_Queens |_s.other_Rooks | _s.other_Bishops | _s.other_Knights | _s.other_Pawns;
    _s.all_pieces = _s.own_pieces | _s.other_pieces;
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
    _s.Queens = _B_Queens;
    _s.Rooks = _B_Rooks;
    _s.Bishops = _B_Bishops;
    _s.Knights = _B_Knights;
    _s.Pawns = _B_Pawns;
    _s.own_pieces = _s.King | _s.Queens | _s.Rooks | _s.Bishops | _s.Knights | _s.Pawns;

    _s.other_King = _W_King;
    _s.other_Queens = _W_Queens;
    _s.other_Rooks = _W_Rooks;
    _s.other_Bishops = _W_Bishops;
    _s.other_Knights = _W_Knights;
    _s.other_Pawns = _W_Pawns;
    _s.other_pieces = _s.other_King | _s.other_Queens | _s.other_Rooks | _s.other_Bishops | _s.other_Knights | _s.other_Pawns;
    _s.all_pieces = _s.own_pieces | _s.other_pieces;
  }
}

} // End namespace C2_chess


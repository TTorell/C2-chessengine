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
#include <cstring>
#include "chessfuncs.hpp"
#include "chesstypes.hpp"
#include "zobrist_bitboard_hash.hpp"
#include "shared_ostream.hpp"
#include "current_time.hpp"

namespace
{

int node_counter = 0;
int hash_hits = 0;
}

namespace C2_chess
{

CurrentTime now;
std::atomic<bool> Bitboard::time_left(false);
Bitboard Bitboard::level_boards[38];

Bitboard::Bitboard() :
    _hash_tag(zero),
    _movelist(),
    _col_to_move(col::white),
    _castling_rights(castling_rights_none),
    _ep_square(zero),
    _material_diff(0),
    _last_move(),
    _checkers(zero),
    _pinners(zero),
    _pinned_pieces(zero),
    _all_pieces(zero),
    _white_pieces(),
    _black_pieces()
{
  _own = &_white_pieces;
  _other = &_black_pieces;
}

Bitboard::Bitboard(const Bitboard& bb) :
    _hash_tag(bb._hash_tag),
    _movelist(bb._movelist),
    _col_to_move(bb._col_to_move),
    _castling_rights(bb._castling_rights),
    _ep_square(bb._ep_square),
    _material_diff(bb._material_diff),
    _last_move(bb._last_move),
    _checkers(bb._checkers),
    _pinners(bb._pinners),
    _pinned_pieces(bb._pinned_pieces),
    _all_pieces(bb._all_pieces),
    _white_pieces(bb._white_pieces),
    _black_pieces(bb._black_pieces)
{
  if(_col_to_move == col::white)
  {
    _own = &_white_pieces;
    _other = &_black_pieces;
  }
  else
  {
    _own = &_black_pieces;
    _other = &_white_pieces;
  }
}

Bitboard& Bitboard::operator=(const Bitboard& from)
{
  //  std::memcpy(this, &from, sizeof(Bitboard));
  _hash_tag = from._hash_tag;
  _movelist = from._movelist;
  _col_to_move = from._col_to_move;
  _castling_rights = from._castling_rights;
  _has_castled[0] = from._has_castled[0];
  _has_castled[1] = from._has_castled[1];
  _ep_square = from._ep_square;
  _material_diff = from._material_diff;
  _last_move = from._last_move;
  _checkers = from._checkers;
  _pinners = from._pinners;
  _pinned_pieces = from._pinned_pieces;
  _all_pieces = from._all_pieces;
  _white_pieces = from._white_pieces;
  _black_pieces = from._black_pieces;
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
  return *this;
}

inline void Bitboard::clear_movelist()
{
  _movelist.clear();
}

// Initializes the Bitboard position from a text string (Forsyth-Edwards Notation)
int Bitboard::read_position(const std::string& FEN_string, bool init_pieces)
{
  //std::cout << "\"" << FEN_string << "\"" << std::endl;
  std::vector<std::string> FEN_tokens = split(FEN_string, ' ');
  if (FEN_tokens.size() != 6)
  {
    std::cerr << "Read error: Number of tokens in FEN string should be 6, but there are " << FEN_tokens.size() << std::endl;
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }

  clear();

  int ri = 8;
  int fi = a;
  for (const char& ch : FEN_tokens[0])
  {
    switch (ch)
    {
      case ' ':
        break;
      case 'K':
        _white_pieces.King = file[fi] & rank[ri];
        break;
      case 'k':
        _black_pieces.King = file[fi] & rank[ri];
        break;
      case 'Q':
        _white_pieces.Queens |= (file[fi] & rank[ri]), _material_diff += 9;
        break;
      case 'q':
        _black_pieces.Queens |= (file[fi] & rank[ri]), _material_diff -= 9;
        break;
      case 'B':
        _white_pieces.Bishops |= (file[fi] & rank[ri]), _material_diff += 3;
        break;
      case 'b':
        _black_pieces.Bishops |= (file[fi] & rank[ri]), _material_diff -= 3;
        break;
      case 'N':
        _white_pieces.Knights |= (file[fi] & rank[ri]), _material_diff += 3;
        break;
      case 'n':
        _black_pieces.Knights |= (file[fi] & rank[ri]), _material_diff -= 3;
        break;
      case 'R':
        _white_pieces.Rooks |= (file[fi] & rank[ri]), _material_diff += 5;
        break;
      case 'r':
        _black_pieces.Rooks |= (file[fi] & rank[ri]), _material_diff -= 5;
        break;
      case 'P':
        _white_pieces.Pawns |= (file[fi] & rank[ri]), _material_diff += 1;
        break;
      case 'p':
        _black_pieces.Pawns |= (file[fi] & rank[ri]), _material_diff -= 1;
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
  _own = &_white_pieces;
  _other = &_black_pieces;
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
  if (init_pieces)
    init_piece_state();
  return 0;
}

void Bitboard::init_piece_state()
{
  clear_movelist();
  _checkers = zero;
  _pinners = zero;
  _pinned_pieces = zero;
  _own->assemble_pieces();
  _other->assemble_pieces();
  _all_pieces = _own->pieces | _other->pieces;
}

void Bitboard::clear_transposition_table()
{
  transposition_table.clear();
}

// This method will run in the timer_thread.
void Bitboard::start_timer(const std::string& max_search_time)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  logfile << "Timer Thread Started." << "\n";

  double time = stod(max_search_time);
  Bitboard::time_left = true;
  while (Bitboard::time_left)
  {
    uint64_t nsec_start = now.nanoseconds();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    uint64_t nsec_stop = now.nanoseconds();
    uint64_t timediff = nsec_stop - nsec_start;
    time -= (double) timediff / 1e6;
    if (time <= 0.0)
    {
      Bitboard::time_left = false;
      break;
    }
  }
  logfile << "Timer Thread Stopped." << "\n";
}

void Bitboard::start_timer_thread(const std::string& max_search_time)
{
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  std::thread timer_thread(start_timer, max_search_time);
  logfile << "Timer Thread Started: " << max_search_time << " milliseconds.\n";
  timer_thread.detach();
}

bool Bitboard::has_time_left()
{
  return time_left;
}

void Bitboard::set_time_left(bool value)
{
  time_left = value;
}

void Bitboard::init_material_evaluation()
{
  const float weight = 1.0F;
  float sum = 0.0;
  sum += std::popcount(_white_pieces.Pawns) * weight;
  sum -= std::popcount(_black_pieces.Pawns) * weight;
  sum += std::popcount(_white_pieces.Knights) * 3.0 * weight;
  sum -= std::popcount(_black_pieces.Knights) * 3.0 * weight;
  sum += std::popcount(_white_pieces.Bishops) * 3.0 * weight;
  sum -= std::popcount(_black_pieces.Bishops) * 3.0 * weight;
  sum += std::popcount(_white_pieces.Rooks) * 5.0 * weight;
  sum -= std::popcount(_black_pieces.Rooks) * 5.0 * weight;
  sum += std::popcount(_white_pieces.Queens) * 9.0 * weight;
  sum -= std::popcount(_black_pieces.Queens) * 9.0 * weight;
  _material_diff = sum;
}

void Bitboard::count_pawns_in_centre(float& sum, float weight) const
{
  sum += weight * (std::popcount(center_squares & _white_pieces.Pawns) - std::popcount(center_squares & _black_pieces.Pawns));
}

void Bitboard::count_castling(float& sum, float weight) const
{
  int counter = 0;
  if (_has_castled[index(col::white)])
    counter++;
  if (_has_castled[index(col::black)])
    counter--;
  sum += counter * weight;
}

void Bitboard::count_development(float& sum, float weight) const
{
  int counter = 0;
  counter -= std::popcount(_white_pieces.Rooks & rook_initial_squares_white);
  counter -= std::popcount(_white_pieces.Knights & knight_initial_squares_white);
  counter -= std::popcount(_white_pieces.Bishops & bishop_initial_squares_white);
  counter += std::popcount(_black_pieces.Rooks & rook_initial_squares_black);
  counter += std::popcount(_black_pieces.Knights & knight_initial_squares_black);
  counter += std::popcount(_black_pieces.Bishops & bishop_initial_squares_black);
  sum += counter * weight;
}

int Bitboard::count_threats_to_square(uint64_t to_square, col color) const
{
  uint64_t possible_attackers;
  uint64_t attacker;
  uint64_t tmp_all_pieces = _all_pieces;
  uint8_t f_idx = file_idx(to_square);

  const Bitpieces& pieces = (color == col::white) ? _white_pieces : _black_pieces;

  int count = 0;
  // Check Pawn-threats
  if (pieces.Pawns)
  {
    if ((f_idx != h) && (pieces.Pawns & ((color == col::white) ? to_square << 7 : to_square >> 9)))
      count++;
    if ((f_idx != a) && (pieces.Pawns & ((color == col::white) ? to_square << 9 : to_square >> 7)))
      count++;
  }

  // Check Knight-threats
  count += std::popcount((adjust_pattern(knight_pattern, to_square) & pieces.Knights));

  // Check King-threats
  count += std::popcount(adjust_pattern(king_pattern, to_square) & pieces.King);

  // Check threats on file and rank
  if (pieces.Queens | pieces.Rooks)
  {
    // Check threats on file and rank
    uint64_t to_ortogonal_squares = ortogonal_squares(to_square);
    possible_attackers = to_ortogonal_squares & (pieces.Queens | pieces.Rooks);
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_ortogonal_squares) & tmp_all_pieces) == zero)
        count++;
    }
  }

  // Check diagonal threats
  if (pieces.Queens | pieces.Bishops)
  {
    uint64_t to_diagonal_squares = diagonal_squares(to_square);
    possible_attackers = to_diagonal_squares & (pieces.Queens | pieces.Bishops);
    while (possible_attackers)
    {
      attacker = popright_square(possible_attackers);
      if ((between(to_square, attacker, to_diagonal_squares, true) & tmp_all_pieces) == zero)
        count++;
    }
  }
  return count;
}

void Bitboard::count_center_control(float& sum, float weight) const
{
  uint64_t center_square;
  int counter = 0;
  uint64_t tmp_center_squares = center_squares;
  while (tmp_center_squares)
  {
    center_square = popright_square(tmp_center_squares);
    counter += count_threats_to_square(center_square, col::white);
    counter -= count_threats_to_square(center_square, col::black);
  }
  sum += counter * weight;
}

float Bitboard::evaluate_position(col col_to_move, uint8_t level) const
{
  if (_movelist.size() == 0)
  {
    // if (square_is_threatened(_own->King, false))
    if (_last_move.properties() & move_props_check) // TODO: Check it this always has been set?
    {
      // This is checkmate, we want to evaluate the quickest way to mate higher
      // so we add/subtract level.
      return (col_to_move == col::white) ? (eval_min + level) : (eval_max - level);
    }
    else
    {
      // This must be stalemate
      return 0.0;
    }
  }
  // Start with a very small number in sum, just so we don't return 0.0 in an
  // equal position. 0.0 is reserved for stalemate.
  float sum = epsilon;
  sum += _material_diff;
  //count_material(sum, 0.95F, ot);
  count_center_control(sum, 0.02F);
  //count_possible_moves(sum, 0.01F, col_to_move); // of doubtful value.
  count_development(sum, 0.05F);
  count_pawns_in_centre(sum, 0.03F);
  count_castling(sum, 0.10F);
  return sum;
}

float Bitboard::max(uint8_t level, uint8_t move_no, float alpha, float beta, int8_t& best_move_index, const uint8_t max_search_level) const
{
  assert(_col_to_move == col::white);
  //std::cout << static_cast<int>(max_search_level) << std::endl;
  float max_value = -101.0; // Must be lower than lowest evaluation
  int8_t dummy_index; // best_move_index is only an output parameter,
  // from min(). It doesn't matter what you put in.
  best_move_index = -1;
  level++;
  //  std::cout << "MAX " << "_col_to_move: " << static_cast<int>(_col_to_move) << " time_left: " <<
  //      time_left << " level: " << static_cast<int>(level) << std::endl;

  // Check if position evaluation is already in the hash_table
  TT_element& element = transposition_table.find(_hash_tag);
  if (element.level != 0)
  {
    // The position was found in the hash_table,
    // but is the evaluation good enough?
    if (element.level <= level)
    {
      hash_hits++;
      best_move_index = element.best_move_index;
      return element.evaluation;
    }
  }
  // If there are no possible moves, the evaluation will check for such things as
  // mate or stalemate which may happen before max_search_level has been reached.
  if (_movelist.size() == 0 || level >= max_search_level)
  {
    if (level >= max_search_level)
      node_counter++;

    // element is a reference to a worse evaluation of this position (don't
    // know if that can happen here because here we are usually at the highest
    // search level) or it is a reference to  a new hash_element, already
    // in the cash, but only default-allocated. Fill it with values;
    element = {best_move_index, // -1, TODO: will this mess up things?
        evaluate_position(col::white, level),
        level};
    return element.evaluation;
  }
  // Collect the best value from all possible moves
  for (uint8_t i = 0; i < static_cast<uint8_t>(_movelist.size()); i++)
  {
    // Copy current board into the preallocated board for this level.
    Bitboard::level_boards[level] = *this;
    // Make the selected move on the "level-board" and ask min() to evaluate it further.
    level_boards[level].make_move(_movelist[i], move_no, (level < max_search_level) ? gentype::all : gentype::captures);
    float tmp_value = level_boards[level].min(level, move_no, alpha, beta, dummy_index, max_search_level);
    // Save the value if it is the "best", so far, from max() point of view.
    if (tmp_value > max_value)
    {
      max_value = tmp_value;
      best_move_index = i;
    }
    // Pruning:
    if (tmp_value >= beta)
    {
      // Look no further. Skip the rest of the branches.
      // They wont produce a better result.
      return max_value;
    }
    if (tmp_value > alpha)
    {
      // Update alpha value for min();
      alpha = tmp_value;
    }
    // Somewhere we have to check if time is up, to interrupt the search.
    // Why not do it here?
    if (!time_left)
    {
      best_move_index = -1;
      return 0.0;
    }
  }
  // Save evaluation of the position to the cash and return
  // the best value among the possible moves.
  element = {best_move_index,
             max_value,
             level};
  return max_value;
}

// min() is naturally very similar to max, but reversed when it comes to evaluations,
// so I've not supplied many comments on this function. See max().
float Bitboard::min(uint8_t level, uint8_t move_no, float alpha, float beta, int8_t& best_move_index, const uint8_t max_search_level) const
{
  assert(_col_to_move == col::black);
  float min_value = 101.0;
  int8_t dummy_index;
  best_move_index = -1;
  level++;
//  std::cout << "MIN " << "_col_to_move: " << static_cast<int>(_col_to_move) << " time_left: " <<
//      time_left << " level: " << static_cast<int>(level) << std::endl;

  TT_element& element = transposition_table.find(_hash_tag);
  if (element.level != 0)
  {
    if (element.level <= level)
    {
      hash_hits++;
      best_move_index = element.best_move_index;
      return element.evaluation;
    }
  }
  if (_movelist.size() == 0 || level >= max_search_level)
  {
    if (level >= max_search_level)
      node_counter++;
    element = {best_move_index, // -1, TODO: will this mess up things?
        evaluate_position(col::black, level),
        level};
    return element.evaluation;
  }
  else
  {
    for (uint8_t i = 0; i < static_cast<uint8_t>(_movelist.size()); i++)
    {
      level_boards[level] = *this;
//      level_boards[level].write(std::cout, outputtype::cmd_line_diagram, col::white);
      level_boards[level].make_move(_movelist[i], move_no, (level < max_search_level) ? gentype::all : gentype::captures);
      float tmp_value = level_boards[level].max(level, move_no, alpha, beta, dummy_index, max_search_level);
      if (tmp_value < min_value)
      {
        min_value = tmp_value;
        best_move_index = i;
      }
      if (tmp_value <= alpha)
      {
        return min_value;
      }
      if (tmp_value < beta)
      {
        beta = tmp_value;
      }
      if (!time_left)
      {
        best_move_index = -1;
        return 0.0;
      }
    }
    element = {best_move_index,
               min_value,
               level};
//    std::cout << "best_move_index: " << static_cast<int>(best_move_index) << " min_value: " << min_value << " level: " << static_cast<int>(level) << std::endl;
    return min_value;
  }
}

inline std::ostream& Bitboard::write_piece(std::ostream& os, uint64_t square) const
{
  if (square & _white_pieces.King)
    os << "\u2654";
  else if (square & _white_pieces.Queens)
    os << "\u2655";
  else if (square & _white_pieces.Rooks)
    os << "\u2656";
  else if (square & _white_pieces.Bishops)
    os << "\u2657";
  else if (square & _white_pieces.Knights)
    os << "\u2658";
  else if (square & _white_pieces.Pawns)
    os << "\u2659";
  else if (square & _black_pieces.King)
    os << "\u265A";
  else if (square & _black_pieces.Queens)
    os << "\u265B";
  else if (square & _black_pieces.Rooks)
    os << "\u265C";
  else if (square & _black_pieces.Bishops)
    os << "\u265D";
  else if (square & _black_pieces.Knights)
    os << "\u265E";
  else if (square & _black_pieces.Pawns)
    os << "\u265F";
  return os;
}

std::ostream& Bitboard::write(std::ostream& os, outputtype wt, col from_perspective) const
{
  switch (wt)
  {
    case outputtype::cmd_line_diagram:
      if (from_perspective == col::white)
      {
        //os << "\n";
        for (int i = 8; i >= 1; i--)
        {
          for (int j = a; j <= h; j++)
          {
            uint64_t square = file[j] & rank[i];
            os << iso_8859_1_to_utf8("|");
            if (square & _all_pieces)
              write_piece(os, square);
            else
              os << ("\u25a1");
          }
          os << iso_8859_1_to_utf8("|") << iso_8859_1_to_utf8(std::to_string(i)) << std::endl;
        }
        os << iso_8859_1_to_utf8(" a b c d e f g h ") << std::endl;
      }
      else // From blacks point of view
      {
        //os << "\n";
        for (int i = 1; i <= 8; i++)
        {
          for (int j = h; j >= a; j--)
          {
            uint64_t square = file[j] & rank[i];
            os << iso_8859_1_to_utf8("|");
            if (square & _all_pieces)
              write_piece(os, square);
            else
              os << ("\u25a1");
          }
          os << iso_8859_1_to_utf8("|") << iso_8859_1_to_utf8(std::to_string(i)) << std::endl;
        }
        os << iso_8859_1_to_utf8(" h g f e d c b a ") << std::endl;
      }
      break;
    default:
      os << iso_8859_1_to_utf8("Sorry: Output type not implemented yet.") << std::endl;
  }
  return os;
}

void Bitboard::clear_node_counter() { node_counter = 0;}
int Bitboard::get_node_counter() const {return node_counter;}
void Bitboard::clear_hash_hits() { hash_hits = 0;}
int Bitboard::get_hash_hits() const {return hash_hits;}

} // End namespace C2_chess


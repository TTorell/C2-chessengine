//============================================================================
// Name        : Bitboard.cpp
// Author      : Torsten
// Version     :
// Copyright   : Your copyright notice
// Description : C++ code for a bitboard-representation of a chess board.
//============================================================================

#include <iostream>
#include <sstream>
#include <string>
#include <atomic>
#include <functional>
#include <cstring>
#include <thread>
#include "chessfuncs.hpp"
#include "chesstypes.hpp"
#include "shared_ostream.hpp"
#include "current_time.hpp"
#include "transposition_table.hpp"
#include "bitboard.hpp"

namespace
{
C2_chess::Search_info search_info;
}

namespace C2_chess
{

Current_time steady_clock;
std::atomic<bool> Bitboard::time_left(false);
//struct Takeback_element Bitboard::takeback_list[N_SEARCH_PLIES_DEFAULT] {};
Game_history Bitboard::history;
TT Bitboard::transposition_table(1000000);
int Bitboard::alpha_move_cash[2][7][64];

Bitboard::Bitboard() :
    _hash_tag(zero), _side_to_move(Color::White), _move_number(1), _castling_rights(castling_rights_none), _ep_square(zero), _material_diff(0), _latest_move(), _iteration_depth(0),
    _checkers(zero), _pinners(zero), _pinned_pieces(zero), _all_pieces(zero), _white_pieces(), _black_pieces(), _own(nullptr), _other(nullptr), _half_move_counter(0)
{
  //std::cerr << "BitBoard Default constructor" << std::endl;
  _own = &_white_pieces;
  _other = &_black_pieces;

}

Bitboard::Bitboard(const Bitboard& bb) :
    _hash_tag(bb._hash_tag), _side_to_move(bb._side_to_move), _move_number(bb._move_number), _castling_rights(bb._castling_rights), _ep_square(bb._ep_square),
    _material_diff(bb._material_diff), _latest_move(bb._latest_move), _iteration_depth(bb._iteration_depth), _checkers(bb._checkers), _pinners(bb._pinners),
    _pinned_pieces(bb._pinned_pieces), _all_pieces(bb._all_pieces), _white_pieces(bb._white_pieces), _black_pieces(bb._black_pieces), _own(nullptr), _other(nullptr),
    _half_move_counter(bb._half_move_counter)
{
  //std::cerr << "BitBoard Copy constructor" << std::endl;
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
}

Bitboard& Bitboard::operator=(const Bitboard& from)
{
  //std::cerr << "BitBoard assignment operator" << std::endl;
  const bool memcopy = false;

  if (memcopy)
  {
    std::memcpy(static_cast<void*>(&this->_hash_tag), static_cast<const void*>(&from._hash_tag), sizeof(Bitboard));
  }
  else
  {
    // State variables:
    _hash_tag = from._hash_tag;
    _side_to_move = from._side_to_move;
    _move_number = from._move_number;
    _castling_rights = from._castling_rights;
    _has_castled[0] = from._has_castled[0];
    _has_castled[1] = from._has_castled[1];
    _ep_square = from._ep_square;
    _material_diff = from._material_diff;
    _latest_move = from._latest_move;
    _half_move_counter = from._half_move_counter;
    // Temporary variables. There's No need to copy them
    // (only used in move generation).
    //_checkers = from._checkers;
    //_pinners = from._pinners;
    //_pinned_pieces = from._pinned_pieces;

    // Piece state variables:
    // Will be taken care of in take_back move.
    _all_pieces = from._all_pieces;
    _white_pieces = from._white_pieces;
    _black_pieces = from._black_pieces;
  }
  // Set pointers
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
  return *this;
}

void Bitboard::init_board_hash_tag()
{
  uint64_t pieces, piece;

  transposition_table.clear();
  _hash_tag = zero;
  pieces = _all_pieces;
  while (pieces)
  {
    piece = popright_square(pieces);
    Piecetype p_type = get_piece_type(piece);
    Color p_color = (piece & _own->pieces) ? _side_to_move : other_color(_side_to_move);
    update_hash_tag(piece, p_color, p_type);
  }

  if ((_castling_rights & castling_right_WK) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_WK];
  if ((_castling_rights & castling_right_WQ) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_WQ];
  if ((_castling_rights & castling_right_BK) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_BK];
  if ((_castling_rights & castling_right_BQ) == 0)
    _hash_tag ^= transposition_table._castling_rights[castling_right_BQ];

  if (_ep_square)
    _hash_tag ^= transposition_table._en_passant_file[file_idx(_ep_square)];

  if (_side_to_move == Color::Black)
    _hash_tag ^= transposition_table._toggle_side_to_move;
}

void Bitboard::assemble_pieces()
{
  _own->assemble_pieces();
  _other->assemble_pieces();
  _all_pieces = _own->pieces | _other->pieces;
}

void Bitboard::init_piece_state()
{
  _checkers = zero;
  _pinners = zero;
  _pinned_pieces = zero;
  assemble_pieces();
}

void Bitboard::init()
{
  init_board_hash_tag();
  history.clear();
  add_position_to_game_history();
  init_piece_state();
}

// Initializes the Bitboard position from a text string (Forsyth-Edwards Notation)
int Bitboard::read_position(const std::string& FEN_string, const bool initialize)
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
        std::cerr << "Read error: unexpected character in FEN input " << ch << "\n";
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
  _side_to_move = Color::White;
  _own = &_white_pieces;
  _other = &_black_pieces;
  if (FEN_tokens[1] == "b")
    update_side_to_move();
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

  std::string half_move_counter = FEN_tokens[4];
  if (half_move_counter.find_first_not_of("0123456789") != std::string::npos || half_move_counter.size() > 2)
  {
    std::cerr << "Read error: unexpected half-move-counter in FEN input " << half_move_counter << "\n";
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }
  uint8_t value = static_cast<uint8_t>(std::stoi(half_move_counter));
  if (value > 50)
  {
    std::cerr << "Read error: unexpected half-move-counter in FEN input: " << value << "\n";
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }
  _half_move_counter = value;

  std::string move_number = FEN_tokens[5];
  if (move_number.find_first_not_of("0123456789") != std::string::npos || move_number.size() > 3)
  {
    std::cerr << "Read error: unexpected move number in FEN input: " << move_number << "\n";
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }
  uint16_t val = static_cast<uint16_t>(std::stoi(move_number));
  if (val > (MAX_HISTORY_PLIES / 2) + (MAX_HISTORY_PLIES % 2))
  {
    std::cerr << "Read error: Too big move number in FEN input: " << val << "\n";
    std::cerr << "FEN-string: " << FEN_string << std::endl;
    return -1;
  }
  _move_number = val;
  if (initialize)
  {
    init_piece_state();
    //find_legal_moves(*get_movelist(0), Gentype::All);
  }
  return 0;
}

void Bitboard::clear_transposition_table()
{
  transposition_table.clear();
}


void Bitboard::update_half_move_counter()
{
  // Update half-move counter for the 50-moves-draw-rule.
  if ((_latest_move.properties() & move_props_capture) || (_latest_move.piece_type() == Piecetype::Pawn))
    _half_move_counter = 0;
  else
    _half_move_counter++;
}

bool Bitboard::is_draw_by_50_moves() const
{
  return _half_move_counter >= 50;
}

// This method will run in the timer-thread.
// max_search_time is in milliseconds
// If time_left is set to false by another thread
// this method returns and the timer-thread dies.
void Bitboard::start_timer(double time)
{
  Bitboard::time_left = true;
  Shared_ostream& logfile = *(Shared_ostream::get_instance());
  logfile << "Timer Thread Started, time: " << time << "\n";

  while (Bitboard::time_left)
  {
    uint64_t nsec_start = steady_clock.nanoseconds();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    uint64_t nsec_stop = steady_clock.nanoseconds();
    uint64_t timediff = nsec_stop - nsec_start;
    time -= static_cast<double>(timediff) / 1e6;
    if (time <= 0.0)
    {
      Bitboard::time_left = false;
      logfile << "Timer thread stopped." << "\n";
      return;
    }
  }
  logfile << "Timer thread: Time is out." << "\n";
}

void Bitboard::start_timer_thread(const double time)
{
  assert(time >= 0);
  Bitboard::time_left = true;
  std::thread timer_thread(start_timer, time);
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
  const auto weight{1.0F};
  auto sum{0.0F};
  sum += static_cast<float>(std::popcount(_white_pieces.Pawns)) * weight;
  sum -= static_cast<float>(std::popcount(_black_pieces.Pawns)) * weight;
  sum += static_cast<float>(std::popcount(_white_pieces.Knights)) * 3.0F * weight;
  sum -= static_cast<float>( std::popcount(_black_pieces.Knights)) * 3.0F * weight;
  sum += static_cast<float>(std::popcount(_white_pieces.Bishops)) * 3.0F * weight;
  sum -= static_cast<float>(std::popcount(_black_pieces.Bishops)) * 3.0F * weight;
  sum += static_cast<float>(std::popcount(_white_pieces.Rooks)) * 5.0F * weight;
  sum -= static_cast<float>(std::popcount(_black_pieces.Rooks)) * 5.0F * weight;
  sum += static_cast<float>(std::popcount(_white_pieces.Queens)) * 9.0F * weight;
  sum -= static_cast<float>(std::popcount(_black_pieces.Queens)) * 9.0F * weight;
  _material_diff = sum;
}

void Bitboard::count_pawns_in_centre(float& sum, float weight) const
{
  sum += weight * static_cast<float>(std::popcount(center_squares & _white_pieces.Pawns) - std::popcount(center_squares & _black_pieces.Pawns));
}

void Bitboard::count_castling(float& sum, float weight) const
{
  int counter = 0;
  if (_has_castled[index(Color::White)])
    counter++;
  if (_has_castled[index(Color::Black)])
    counter--;
  sum += static_cast<float>(counter) * weight;
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
  sum += static_cast<float>(counter) * weight;
}

int Bitboard::walk_into_center(const uint64_t from_sq, const int n_center_sqs, std::function<void(uint64_t&)> shift_func) const
{
  auto count = 0;
  auto all_pieces = _white_pieces.pieces | _black_pieces.pieces;
  uint64_t sq = from_sq;
  for (shift_func(sq); count < n_center_sqs; shift_func(sq))
  {
    count += (sq & center_squares) ? 1 : 0;
    if (sq & all_pieces)
      return count;
  }
  return count;
}

int Bitboard::count_threats_to_center_squares(const Bitpieces& pieces, const uint64_t pawn_pattern) const
{

  auto count = 0;
  // Check Pawn-threats
  count += std::popcount(pawn_pattern & pieces.Pawns);

  // Check Knight-threats
  count += std::popcount(knight_center_control_pattern1 & pieces.Knights);
  count += 2 * std::popcount(knight_center_control_pattern2 & pieces.Knights);

  // Check King-threats
  if (king_center_control_pattern1 & pieces.King)
  {
    count++;
  }
  if (king_center_control_pattern2 & pieces.King)
  {
    count += 2;
  }

  auto squares = rook_center_control_pattern & (pieces.Queens | pieces.Rooks);
  while (squares)
  {
    auto from_square = popright_square(squares);
    if (from_square & west_of_center)
    {
      count += walk_into_center(from_square, 2, &go_east);
    }
    else if (from_square & east_of_center)
    {
      count += walk_into_center(from_square, 2, &go_west);
    }
    else if (from_square & south_of_center)
    {
      count += walk_into_center(from_square, 2, &go_north);
    }
    else
    {
      count += walk_into_center(from_square, 2, &go_south);
    }
  }
  squares = bishop_center_control_pattern1 & (pieces.Queens | pieces.Bishops);
  while (squares)
  {
    auto from_square = popright_square(squares);
    if (from_square & bishop_south_west_of_center)
    {
      count += walk_into_center(from_square, 1, &go_north_east);
    }
    else if (from_square & bishop_north_west_of_center)
    {
      count += walk_into_center(from_square, 1, &go_south_east);
    }
    else if (from_square & bishop_south_east_of_center)
    {
      count += walk_into_center(from_square, 1, &go_north_west);
    }
    else
    {
      count += walk_into_center(from_square, 1, &go_south_west);
    }
  }
  squares = bishop_center_control_pattern2 & (pieces.Queens | pieces.Bishops);
  while (squares)
  {
    auto from_square = popright_square(squares);
    if (from_square & south_west_of_center)
    {
      count += walk_into_center(from_square, 2, &go_north_east);
    }
    else if (from_square & north_west_of_center)
    {
      count += walk_into_center(from_square, 2, &go_south_east);
    }
    else if (from_square & south_east_of_center)
    {
      count += walk_into_center(from_square, 2, &go_north_west);
    }
    else
    {
      count += walk_into_center(from_square, 2, &go_south_west);
    }
  }

  // Check pieces inside the center
  count += 3 * std::popcount(center_squares & (pieces.King | pieces.Queens));
  count += 2 * std::popcount(center_squares & pieces.Rooks);
  count += std::popcount(center_squares & pieces.Bishops);

  // Pawns in center have already been handled and knights
  // in the center threatens no center squares.

  return count;
}

int Bitboard::count_passers_and_isolanis() const
{
  int count = 0;
  uint64_t pawn_square;
  uint64_t pattern;
  uint64_t pawns = _white_pieces.Pawns;
  while (pawns)
  {
    pawn_square = popright_square(pawns);
    // double pawns
    count -= std::popcount(to_file(pawn_square) & _white_pieces.Pawns) - 1;
    // isolated pawns
    count -= (isolani_pattern[file_idx(pawn_square)] & _white_pieces.Pawns) ? 0 : 1;
    // passed_pawns
    pattern = adjust_passer_pattern(passed_pawn_pattern_W, pawn_square, bit_idx(e2_square));
    count += (pattern & _black_pieces.Pawns) ? 0 : rank_idx(pawn_square) - 1;
  }
  pawns = _black_pieces.Pawns;
  while (pawns)
  {
    pawn_square = popright_square(pawns);
    count += std::popcount(to_file(pawn_square) & _black_pieces.Pawns) - 1;
    pattern = adjust_isolani_pattern(isolated_pawn_pattern, pawn_square);
    count += (pattern & _black_pieces.Pawns) ? 0 : 1;
    pattern = adjust_passer_pattern(passed_pawn_pattern_B, pawn_square, bit_idx(e7_square));
    count -= (pattern & _white_pieces.Pawns) ? 0 : 8 - rank_idx(pawn_square);
  }
  return count;
}

float Bitboard::evaluate_empty_movelist(int search_ply) const
{
  if (_latest_move.properties() & move_props_check)
  {
    // This is checkmate, we want to evaluate the quickest way to mate higher
    // so we add/subtract level.
    return (_side_to_move == Color::White) ? (eval_min + static_cast<float>(search_ply)) : (eval_max - static_cast<float>(search_ply));
  }
  else
  {
    // This must be stalemate
    return 0.0;
  }
}

float Bitboard::evaluate_position() const
{
// Start with a very small number in sum, just so we don't return 0.0 in an
// equal position. 0.0 is reserved for stalemate.
  auto sum = epsilon;
  sum += _material_diff;

  //count center control
  sum += static_cast<float>(count_threats_to_center_squares(_white_pieces, pawn_center_control_W_pattern) - count_threats_to_center_squares(_black_pieces, pawn_center_control_B_pattern)) * 0.02F;
  count_development(sum, 0.05F);
  count_pawns_in_centre(sum, 0.03F);
  count_castling(sum, 0.10F);
  sum += static_cast<float>(count_passers_and_isolanis()) * 0.02F;
  return sum;
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

std::ostream& Bitboard::write(std::ostream& os, const Color from_perspective) const
{
  if (from_perspective == Color::White)
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
  return os;
}

// TODO: Put this outside Bitboard?
std::ostream& Bitboard::write_movelist(const std::deque<Bitmove> & movelist, std::ostream& os, bool same_line) const
{
  if (movelist.size() > 0)
  {
    bool first = true;
    for (const Bitmove& m : movelist)
    {
      if (same_line)
      {
        if (!first)
          os << " ";
        first = false;
      }
      else
      {
        if (!first)
          os << std::endl;
        first = false;
      }
      os << m;
    }
    os << std::endl;
  }
  return os;
}

void Bitboard::get_pv_line(std::vector<Bitmove>& pv_line) const
{
  pv_line.clear();
  Bitboard bb = *this;
  Takeback_state tb_state_dummy;
  for (int i = 0; i < 20; i++)
  {
    TT_elem& tte = transposition_table.find(bb._hash_tag);
    if (!tte._best_move.is_valid() || tte._search_depth != _iteration_depth - i)
      break;
    bb.new_make_move(tte._best_move, tb_state_dummy, dont_update_history);
    pv_line.push_back(bb._latest_move);
  }
}

void Bitboard::clear_search_info()
{
  memset(&search_info, 0, sizeof(Search_info));
}

void Bitboard::clear_search_vars()
{
  memset(alpha_move_cash, 0, 2 * 7 * 64);
  memset(_beta_killers, 0, 2 * 64);
}

Search_info& Bitboard::get_search_info() const
{
  return search_info;
}

unsigned int Bitboard::perft_test(int search_ply, const int max_search_depth)
{
  search_ply++;

// Perft-test only counts leaf-nodes on highest depth, not leaf-nodes which happens
// on lower depth because of mate or stalemate.
  if (search_ply >= max_search_depth)
  {
    return ++search_info.leaf_node_counter;
  }

  Takeback_state tb_state;
  list_t movelist;
  find_legal_moves(movelist, Gentype::All);
// Collect the best value from all possible moves
  for (std::size_t i = 0; i < movelist.size(); i++)
  {
    // Make the selected move and ask min() to evaluate it further.
    new_make_move(movelist[i], tb_state);
    perft_test(search_ply, max_search_depth);
    takeback_latest_move(tb_state);
  }

  return search_info.leaf_node_counter;
}

Shared_ostream& Bitboard::write_search_info(Shared_ostream& logfile) const
{
  logfile << "Evaluated on depth:" << static_cast<int>(search_info.max_search_depth) << " " << static_cast<int>(search_info.node_counter) << " nodes in " << search_info.time_taken
          << " milliseconds.\n";
  std::stringstream ss;
  std::vector<Bitmove> pv_line;
  get_pv_line(pv_line);
  write_vector(pv_line, ss, true);
  logfile << "PV_line: " << ss.str();
  logfile << search_info << "\n";
  return logfile;
}

void Bitboard::takeback_from_state(const Takeback_state& tb_state)
{
  _hash_tag = tb_state.hash_tag;
  _castling_rights = tb_state.castling_rights;
  _half_move_counter = tb_state.half_move_counter;
  _ep_square = tb_state.ep_square;
  _has_castled[index(Color::White)] = tb_state.has_castled_w;
  _has_castled[index(Color::Black)] = tb_state.has_castled_b;
  _latest_move = tb_state.latest_move;
}

void Bitboard::save_in_takeback_state(Takeback_state& tb_state) const
{
  tb_state.hash_tag = _hash_tag;
  tb_state.castling_rights = _castling_rights;
  tb_state.half_move_counter = _half_move_counter;
  tb_state.ep_square = _ep_square;
  tb_state.has_castled_w = _has_castled[index(Color::White)];
  tb_state.has_castled_b = _has_castled[index(Color::Black)];
  tb_state.latest_move = _latest_move;
}

float Bitboard::Quiesence_search(float alpha, float beta, const int search_ply, const int max_search_ply)
{
  assert(beta > alpha);

//  std::cout << "Q" << std::endl;
  search_info.node_counter++;

  if (history.is_threefold_repetition() || is_draw_by_50_moves())
  {
    return 0.0F;
  }

  auto score = (_side_to_move == Color::White) ? evaluate_position() : -evaluate_position();

  if (search_ply >= max_search_ply)
  {
    return score;
  }

  if (score >= beta)
  {
    return beta;
  }

  if (score > alpha)
  {
    alpha = score;
  }

  Takeback_state tb_state;
  list_t movelist;
  find_legal_moves(movelist, Gentype::Captures_and_Promotions);

  auto move_score = -infinite;
  // Collect the best value from all possible "capture-moves"
  for (std::size_t i = 0; i < movelist.size(); i++)
  {

    if (search_ply > search_info.highest_search_ply)
      search_info.highest_search_ply = search_ply;

    // Make the selected move and ask  Quiesence_search() to evaluate it further.
    new_make_move(movelist[i], tb_state);
    //std::cout << search_ply << level_boards[search_ply].last_move() << std::endl;
    move_score = -Quiesence_search(-beta, -alpha, search_ply + 1, max_search_ply);

    // Restore game history, state and search_ply to "current position".
    takeback_latest_move(tb_state);

    // Pruning:
    if (move_score > alpha)
    {
      if (move_score >= beta)
      {
        // Beta cut-off.
        // Look no further. Skip the rest of the branches.
        // The other player won't choose this path anyway.
        // TODO: what about best_move;
        search_info.beta_cutoffs++;
        if (i == 0)
        {
          search_info.first_beta_cutoffs++;
        }
        return beta;
      }
      // Update alpha value.
      // We have found a better move.
      alpha = move_score;
    }
  }
  return alpha;
}

bool Bitboard::not_likely_in_zugswang()
{
  // If we have at least one "big piece" left, we are likely not in zugzwang.
  if (_own->pieces & ~_own->King & ~_own->Pawns)
  {
    return true;
  }
  return false;
}

bool Bitboard::nullmove_conditions_OK(const int search_depth)
{
  if (search_depth >= 4 &&
      (_latest_move.properties() & move_props_check) == 0 &&
      not_likely_in_zugswang())
  {
    return true;
  }
  return false;
}

// // Search algorithm: Negamax with alpha-beta-pruning
// float Bitboard::negamax_with_pruning(float alpha, float beta, Bitmove& best_move, const uint8_t search_depth, const bool nullmove_pruning)
// {

//   assert(beta > alpha);

//   best_move = NO_MOVE;
//   if (_search_ply >= search_depth) // TODO: isn't a check for equality enough?
//   {
//     // Quiescence search.
//     // It takes over the searching and the node counting from here,
//     search_info.leaf_node_counter++;
//     auto evaluation = Quiesence_search(alpha, beta, N_SEARCH_PLIES_DEFAULT);
//     //std::cout << "Q search " << evaluation << std::endl;
//     return evaluation;
//   }

//   search_info.node_counter++;

//   if (history.is_threefold_repetition())
//   {
//     best_move = DRAW_BY_THREEFOLD_REPETITION;
//     return 0.0;
//   }

//   if (is_draw_by_50_moves())
//   {
//     best_move = DRAW_BY_50_MOVES_RULE;
//     return 0.0;
//   }

//   // Check if position and evaluation is already in the hash_table
//   TT_elem& element = transposition_table.find(_hash_tag);
//   if (element.is_valid(_hash_tag))
//   {
//     // The position was found in the transposition table,
//     // but is the evaluation good enough?
//     if (element._search_ply <= _search_ply)
//     {
//       search_info.hash_hits++;
//       best_move = element._best_move;
//       return element._best_move._evaluation;
//     }
//   }

//   // Nullmove heuristic.
//   // Don't make a move, just hand over the turn to move to the opponent,
//   // search on a lower depth to see if the opponent can improve beta,
//   // otherwise we can cut off the search-tree here.
//   // (Doesn't work if we're in check. Gives wrong result if we're in zugzwang,
//   // when our best alternative would in fact be to, illegally, do nothing.)
//   Takeback_state tb_state;
//   float nullmove_score = -infinite; // Must be lower than lowest evaluation

//   if (nullmove_pruning && nullmove_conditions_OK(search_depth))
//   {
//     make_nullmove(tb_state, do_update_history); // Attention! Increases _search_ply
//     nullmove_score = -negamax_with_pruning(-beta, -beta + 1.0F, best_move, search_depth - (_search_ply - 1) - 4, no_nullmove_pruning);
//     takeback_null_move(tb_state, dont_update_history);
//     if (nullmove_score >= beta && fabs(nullmove_score) < 90.0F) // not mate
//     {
//       search_info.nullmove_cutoffs++;
//       return beta;
//     }
//   }

//   // Now it's time to generate all possible moves in the position
//   Bitmove best_move_dummy;
//   list_t movelist;
//   find_legal_moves(movelist, Gentype::All);

//   // If there are no possible moves, evaluate_empty_movelist() will check for such things as
//   // mate or stalemate which may happen before max_search_ply has been reached.
//   if (movelist.size() == 0)
//   {
//     // ---------------------------------------
//     float evaluation = (_side_to_move == Color::White) ? evaluate_empty_movelist(_search_ply) : -evaluate_empty_movelist(_search_ply);
//     element.set(_hash_tag, UNDEFINED_MOVE, evaluation, _search_ply);
//     // ---------------------------------------
//     return element._best_move._evaluation;
//   }

//   float move_score = -infinite; // Must be lower than lowest evaluation
//   // Collect the best value from all possible moves
//   for (std::size_t i = 0; i < movelist.size(); i++)
//   {
//     // Make the selected move and ask min() to evaluate it further.
//     //std::cout << static_cast<int>(search_ply) << " " << magic_enum::enum_name(_side_to_move) << " " << movelist[i] << std::endl;
//     new_make_move(movelist[i], tb_state);
//     move_score = -negamax_with_pruning(-beta, -alpha, best_move_dummy, search_depth, nullmove_pruning);

//     // Restore game history and state to current position.
//     takeback_latest_move(tb_state);

//     // Pruning:
//     if (move_score >= beta)
//     {
//       // Beta cut-off.
//       // Look no further. Skip the rest of the branches.
//       // The other player won't choose this path anyway.
//       // TODO: what about best_move;
//       search_info.beta_cutoffs++;
//       if (i == 0)
//         search_info.first_beta_cutoffs++;
//       // Save beta-killers for next move ordering.
//       if ((movelist[i].properties() & move_props_capture) == 0 && (movelist[i].properties() & move_props_promotion) == 0)
//       {
//         _beta_killers[1][_search_ply] = _beta_killers[0][_search_ply];
//         _beta_killers[0][_search_ply] = movelist[i];
//       }
//       return beta;
//     }
//     if (move_score > alpha)
//     {
//       // Update alpha value.
//       // We have found a better move.
//       best_move = (movelist)[i];
//       best_move._evaluation = move_score;
//       alpha = move_score;
//       alpha_move_cash[index(_side_to_move)][index(best_move.piece_type())][bit_idx(best_move.from())] += search_depth - _search_ply;
//     }

//     // Somewhere we have to check if time is up, or if the chess-engine
//     // has received a "stop"-command, to interrupt the search.
//     // This seems to be a good place to do that.
//     if (!time_left)
//     {
//       best_move = SEARCH_HAS_BEEN_INTERRUPTED;
//       search_info.search_interrupted = true;
//       return 0.0;
//     }
//   }

//   // Save evaluation of the position to the TT-cash and return the best
//   // value among the possible moves.
//   // If we haven't found any best_move (better than the original alpha)
//   // then best_move isn't valid.
//   if (best_move.is_valid())
//   {
//     element.set(_hash_tag, best_move, alpha, _search_ply);
//   }
//   return alpha;
// }

// Search algorithm: Negamax with alpha-beta-pruning
float Bitboard::new_negamax_with_pruning(float alpha, float beta, Bitmove& best_move, const int search_depth, const bool nullmove_pruning)
{

  assert(beta > alpha);
  //assert(nullmove_pruning == false);
  // if (nullmove_pruning)
  // {
  //   std::cout << "NULLMOVEPRUNING !!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  // }

  int search_ply = _iteration_depth - search_depth;

  best_move = NO_MOVE;
  if (search_depth <= 0) // TODO: isn't a check for equality enough?
  {
    // Quiescence search.
    // It takes over the searching and the node counting from here,
    search_info.leaf_node_counter++;
    auto evaluation = Quiesence_search(alpha, beta, search_ply, MAX_N_SEARCH_PLIES_DEFAULT);
    //std::cout << "Q search " << evaluation << std::endl;
    return evaluation;
  }

  search_info.node_counter++;

  if (history.is_threefold_repetition())
  {
    best_move = DRAW_BY_THREEFOLD_REPETITION;
    return 0.0;
  }

  if (is_draw_by_50_moves())
  {
    best_move = DRAW_BY_50_MOVES_RULE;
    return 0.0;
  }

  // Check if position and evaluation is already in the hash_table
  TT_elem& element = transposition_table.find(_hash_tag);
  if (element.is_valid(_hash_tag))
  {
    // The position was found in the transposition table,
    // but is the evaluation good enough?
    if (element._search_depth >= search_depth)
    {
      search_info.hash_hits++;
      best_move = element._best_move;
      return element._best_move._evaluation;
    }
  }

  // Nullmove heuristics.
  // Don't make a move, just hand over the turn to move to the opponent,
  // search on a lower depth to see if the opponent can improve beta,
  // otherwise we can cut off the search-tree here.
  // (Doesn't work if we're in check. Gives wrong result if we're in zugzwang,
  // when our best alternative would in fact be to, illegally, do nothing.)
  Takeback_state tb_state;

  if (nullmove_pruning && search_ply > 0 && nullmove_conditions_OK(search_depth))
  {
    make_nullmove(tb_state, do_update_history);
    auto nullmove_score = -new_negamax_with_pruning(-beta, -beta + 1.0F, best_move, search_depth - 4, no_nullmove_pruning);
    takeback_null_move(tb_state, do_update_history);
    if (nullmove_score >= beta && fabsf(nullmove_score) < 90.0F) // not mate
    {
      search_info.nullmove_cutoffs++;
      return beta;
    }
  }

  // Now it's time to generate all possible moves in the position
  Bitmove best_move_dummy;
  list_t movelist;
  find_legal_moves(movelist, Gentype::All, search_depth);

  // If there are no possible moves, evaluate_empty_movelist() will check for such things as
  // mate or stalemate which may happen before max_search_ply has been reached.
  if (movelist.size() == 0)
  {
    // ---------------------------------------
    float evaluation = (_side_to_move == Color::White) ? evaluate_empty_movelist(search_ply) : -evaluate_empty_movelist(search_ply);
    element.set(_hash_tag, UNDEFINED_MOVE, evaluation, search_depth);
    // ---------------------------------------
    return element._best_move._evaluation;
  }

  float move_score = -infinite; // Must be lower than lowest evaluation
  // Collect the best value from all possible moves
  for (std::size_t i = 0; i < movelist.size(); i++)
  {
    // Make the selected move and ask min() to evaluate it further.
    //std::cout << static_cast<int>(search_depth) << " " << magic_enum::enum_name(_side_to_move) << " " << movelist[i] << std::endl;
    new_make_move(movelist[i], tb_state);
    move_score = -new_negamax_with_pruning(-beta, -alpha, best_move_dummy, search_depth - 1, nullmove_pruning);

    // Restore game history and state to current position.
    takeback_latest_move(tb_state);

    // Pruning:
    if (move_score >= beta)
    {
      // Beta cut-off.
      // Look no further. Skip the rest of the branches.
      // The other player won't choose this path anyway.
      // TODO: what about best_move;
      search_info.beta_cutoffs++;
      if (i == 0)
        search_info.first_beta_cutoffs++;
      // Save beta-killers for next move ordering.
      if ((movelist[i].properties() & move_props_capture) == 0 && (movelist[i].properties() & move_props_promotion) == 0)
      {
        _beta_killers[1][search_ply] = _beta_killers[0][search_ply];
        _beta_killers[0][search_ply] = movelist[i];
      }
      return beta;
    }

    if (move_score > alpha)
    {
      // Update alpha value.
      // We have found a better move.
      best_move = (movelist)[i];
      best_move._evaluation = move_score;
      alpha = move_score;
      alpha_move_cash[index(_side_to_move)][index(best_move.piece_type())][bit_idx(best_move.from())] +=  search_depth;
    }

    // Somewhere we have to check if time is up, or if the chess-engine
    // has received a "stop"-command, to interrupt the search.
    // This seems to be a good place to do that.
    if (!time_left)
    {
      best_move = SEARCH_HAS_BEEN_INTERRUPTED;
      search_info.search_interrupted = true;
      return 0.0;
    }
  }

  // Save evaluation of the position to the TT-cash and return the best
  // value among the possible moves.
  // If we haven't found any best_move (better than the original alpha)
  // then best_move isn't valid.
  if (best_move.is_valid())
  {
    element.set(_hash_tag, best_move, alpha, search_depth);
  }
  return alpha;
}

} // End namespace C2_chess

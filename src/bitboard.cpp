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
#include "shared_ostream.hpp"
#include "current_time.hpp"
#include "transposition_table.hpp"

namespace
{
C2_chess::Search_info search_info;
}

namespace C2_chess
{

Current_time steady_clock;
std::atomic<bool> Bitboard::time_left(false);
struct Takeback_element Bitboard::takeback_list[N_SEARCH_BOARDS_DEFAULT]{};
Bitboard Bitboard::search_boards[N_SEARCH_BOARDS_DEFAULT];
Game_history Bitboard::history;

Bitboard::Bitboard() :
    _hash_tag(zero),
    _side_to_move(Color::White),
    _move_number(1),
    _castling_rights(castling_rights_none),
    _ep_square(zero),
    _material_diff(0),
    _last_move(),
    _checkers(zero),
    _pinners(zero),
    _pinned_pieces(zero),
    _all_pieces(zero),
    _white_pieces(),
    _black_pieces(),
    _own(nullptr),
    _other(nullptr),
    _half_move_counter(0)
{
  //std::cerr << "BitBoard Default constructor" << std::endl;
  _own = &_white_pieces;
  _other = &_black_pieces;
}

Bitboard::Bitboard(const Bitboard& bb) :
    _hash_tag(bb._hash_tag),
    _side_to_move(bb._side_to_move),
    _move_number(bb._move_number),
    _castling_rights(bb._castling_rights),
    _ep_square(bb._ep_square),
    _material_diff(bb._material_diff),
    _last_move(bb._last_move),
    _checkers(bb._checkers),
    _pinners(bb._pinners),
    _pinned_pieces(bb._pinned_pieces),
    _all_pieces(bb._all_pieces),
    _white_pieces(bb._white_pieces),
    _black_pieces(bb._black_pieces),
    _own(nullptr),
    _other(nullptr),
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
    std::memcpy(static_cast<void*>(&this->_hash_tag),
                static_cast<const void*>(&from._hash_tag),
                sizeof(Bitboard));
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
    _last_move = from._last_move;
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
    _hash_tag ^= transposition_table._black_to_move;
}

void Bitboard::init_piece_state()
{
  //TODO: REMOVE movelist.clear();
  _checkers = zero;
  _pinners = zero;
  _pinned_pieces = zero;
  _own->assemble_pieces();
  _other->assemble_pieces();
  _all_pieces = _own->pieces | _other->pieces;
}

void Bitboard::init()
{
  init_board_hash_tag();
  history.clear();
  add_position_to_game_history();
  init_piece_state();
  //find_legal_moves(*get_movelist(0), Gentype::All);
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


void Bitboard::clear_transposition_table(map_tag map)
{
  transposition_table.clear(map);
}

void Bitboard::switch_tt_tables()
{
  transposition_table.switch_maps();
}

//TODO: REMOVE
//inline void Bitboard::clear_movelist(atd::deque<Bitmove>& movelist)
//{
//  movelist.clear();
//}

void Bitboard::update_half_move_counter()
{
  // Update half-move counter for the 50-moves-drawing-rule.
  if ((_last_move.properties() & move_props_capture) || (_last_move.piece_type() == Piecetype::Pawn))
    _half_move_counter = 0;
  else
    _half_move_counter++;
}

void Bitboard::set_half_move_counter(uint8_t half_move_counter)
{
  _half_move_counter = half_move_counter;
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
  if (_has_castled[index(Color::White)])
    counter++;
  if (_has_castled[index(Color::Black)])
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

int Bitboard::count_threats_to_square(uint64_t to_square, Color side) const
{
  uint64_t possible_attackers;
  uint64_t attacker;
  uint64_t tmp_all_pieces = _all_pieces;
  uint8_t f_idx = file_idx(to_square);

  const Bitpieces& pieces = (side == Color::White) ? _white_pieces : _black_pieces;

  int count = 0;
  // Check Pawn-threats
  if (pieces.Pawns)
  {
    if ((f_idx != h) && (pieces.Pawns & ((side == Color::White) ? to_square << 7 : to_square >> 9)))
      count++;
    if ((f_idx != a) && (pieces.Pawns & ((side == Color::White) ? to_square << 9 : to_square >> 7)))
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
    counter += count_threats_to_square(center_square, Color::White);
    counter -= count_threats_to_square(center_square, Color::Black);
  }
  sum += counter * weight;
}

float Bitboard::evaluate_position(const bool movelist_is_empty, Color col_to_move, uint8_t search_ply, bool evaluate_zero_moves) const
{
  if (evaluate_zero_moves && movelist_is_empty)
  {
    // if (square_is_threatened(_own->King, false))
    if (_last_move.properties() & move_props_check) // TODO: Check if this always has been set?
    {
      // This is checkmate, we want to evaluate the quickest way to mate higher
      // so we add/subtract level.
      return (col_to_move == Color::White) ? (eval_min + search_ply) : (eval_max - search_ply);
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
std::ostream& Bitboard::write_movelist(const list_ref movelist, std::ostream& os, bool same_line) const
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
  //TODO: Is this righ?
  list_t movelist;
  pv_line.clear();
  Bitboard bb = *this;
  for (int i = 0; i < 20; i++)
  {
    TT_element& tte = transposition_table.find(bb._hash_tag);
    if (!tte.best_move.is_valid())
      break;
    bb.make_move(movelist, tte.best_move, Gentype::All, dont_update_history);
    pv_line.push_back(bb._last_move);
  }
}

void Bitboard::clear_search_info()
{
  memset(&search_info, 0, sizeof(Search_info));
}

Search_info& Bitboard::get_search_info() const
{
  return search_info;
}

unsigned int Bitboard::perft_test(uint8_t search_ply, uint8_t max_search_depth) const
{
  list_ptr next_movelist;
  list_ptr movelist = get_movelist(search_ply);

  search_ply++;
  next_movelist = get_movelist(search_ply);
  // Perft-test only counts leaf-nodes on highest depth, not leaf-nodes which happens
  // on lower depth because of mate or stalemate.
  if (search_ply >= max_search_depth)
  {
    return ++search_info.leaf_node_counter;
  }

  // Collect the best value from all possible moves
  for (uint8_t i = 0; i < static_cast<uint8_t>(movelist->size()); i++)
  {
    // Copy current board into the preallocated board for this search_ply.
    Bitboard::search_boards[search_ply] = *this;

    // Make the selected move on the "ply-board" and ask min() to evaluate it further.
    search_boards[search_ply].make_move(*next_movelist, (*movelist)[i], Gentype::All);
    search_boards[search_ply].perft_test(search_ply, max_search_depth);
    movelist = get_movelist(search_ply-1);
  }

  return search_info.leaf_node_counter;
}

Shared_ostream& Bitboard::write_search_info(Shared_ostream& logfile) const
{
  logfile << "Evaluated on depth:" << static_cast<int>(search_info.max_search_depth) << " "
          << static_cast<int>(search_info.leaf_node_counter)
          << " nodes in " << search_info.time_taken
          << " milliseconds.\n";
  std::stringstream ss;
  std::vector<Bitmove> pv_line;
  get_pv_line(pv_line);
  write_vector(pv_line, ss, true);
  logfile << "PV_line: " << ss.str();
  logfile << search_info << "\n";
  return logfile;
}

void Bitboard::takeback_from_state(Takeback_state& state)
{
  _hash_tag = state._hash_tag;
  _castling_rights = state._castling_rights;
  _half_move_counter = state._half_move_counter;
  _side_to_move = state._side_to_move;
  _move_number = state._move_number;
  _has_castled[0] = state._has_castled_0;
  _has_castled[1] = state._has_castled_1;
  _ep_square = state._ep_square;
  _material_diff = state._material_diff;
  _last_move = state._last_move;
}

float Bitboard::Quiesence_search(uint8_t search_ply, float alpha, float beta, uint8_t max_search_ply)
{
  assert(beta > alpha);

  // Current quiescence-movelist
  list_ptr movelist = get_movelist_Q(search_ply);
  // TODO: s this right?
  takeback_list[search_ply].state_Q = {get_movelist_Q(search_ply),
                                       _hash_tag,
                                       _castling_rights,
                                       _half_move_counter,
                                       _side_to_move,
                                       _move_number,
                                       _has_castled[0],
                                       _has_castled[1],
                                       _ep_square,
                                       _material_diff,
                                       _last_move};

  search_info.node_counter++;

  if (history.is_threefold_repetition() || is_draw_by_50_moves())
  {
    return 0.0;
  }

  float score = (_side_to_move == Color::White) ? evaluate_position(movelist->size() == 0, _side_to_move, search_ply, dont_evaluate_zero_moves) :
                                                  -evaluate_position(movelist->size() == 0, _side_to_move, search_ply, dont_evaluate_zero_moves);

  if (movelist->size() == 0)
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

  search_ply++;
  // Get a pointer to next movelist
  list_ptr next_movelist = get_movelist_Q(search_ply);

  float move_score = -infinity;

  // Collect the best value from all possible "capture-moves"
  for (uint8_t i = 0; i < static_cast<uint8_t>(movelist->size()); i++)
  {

    if (search_ply > search_info.highest_search_ply)
       search_info.highest_search_ply = search_ply;

    // Copy current board into the preallocated board for this search_ply.
    Bitboard::search_boards[search_ply] = *this;

    // Save history state for current position.
    History_state saved_history_state = history.get_state();

    // Make the selected move on the "ply-board" and ask min() to evaluate it further.
    search_boards[search_ply].make_move(*next_movelist, (*movelist)[i], Gentype::Captures_and_Promotions);
    //std::cout << search_ply << level_boards[search_ply].last_move() << std::endl;
    move_score = -search_boards[search_ply].Quiesence_search(search_ply, -beta, -alpha, max_search_ply);

    // Restore game history, state and local movelist to current position.
    history.takeback_moves(saved_history_state);
    takeback_from_state(takeback_list[search_ply - 1].state_Q);

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

// Search algorithm: Negamax with alpha-beta-pruning
float Bitboard::negamax_with_pruning(uint8_t search_ply, float alpha, float beta, Bitmove& best_move, const uint8_t search_depth)
{

  assert(beta > alpha);
  search_info.node_counter++;
  float move_score = -infinity; // Must be lower than lowest evaluation


  // Current movelist
  list_ptr movelist = get_movelist(search_ply);
  takeback_list[search_ply].state_S = {get_movelist(search_ply),
                                     _hash_tag,
                                     _castling_rights,
                                     _half_move_counter,
                                     _side_to_move,
                                     _move_number,
                                     _has_castled[0],
                                     _has_castled[1],
                                     _ep_square,
                                     _material_diff,
                                     _last_move};

  // Next search_ply and next_movelist:
  search_ply++;
  list_ptr next_movelist = get_movelist(search_ply);

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
  TT_element& element = transposition_table.find(_hash_tag);
  if (element.is_initialized())
  {
    // The position was found in the transposition table,
    // but is the evaluation good enough?
    if (element.search_ply <= search_ply)
    {
      search_info.hash_hits++;
      best_move = element.best_move;
      return element.best_move._evaluation;
    }
  }

  // If there are no possible moves, the evaluation will check for such things as
  // mate or stalemate which may happen before max_search_ply has been reached.
  if (movelist->size() == 0)
  {
    // ---------------------------------------
    best_move = NO_MOVE;
    element.best_move = NO_MOVE;
    element.best_move._evaluation = (_side_to_move == Color::White) ? evaluate_position(movelist->size() == 0, _side_to_move, search_ply) :
                                                                      -evaluate_position(movelist->size() == 0, _side_to_move, search_ply);
    element.search_ply = search_ply;
    // ---------------------------------------
    return element.best_move._evaluation;
  }

  if (search_ply == search_depth + 1)
  {
    // Qiescence search.
    // It takes over the searching and the node counting from here,
    search_info.leaf_node_counter++;
    best_move = NO_MOVE;
    element.best_move = NO_MOVE;

    // Quiesence_search will increment the search_ply and the node
    // counting, so we must subtract one in input parameters here.
    // We also have to generate the first movelist for the Qsearch.
    search_info.node_counter--;
    find_legal_moves(*get_movelist_Q(search_ply - 1), Gentype::Captures_and_Promotions);
    element.best_move._evaluation = Quiesence_search(search_ply - 1, alpha, beta, N_SEARCH_BOARDS_DEFAULT);
    element.search_ply = search_ply;
    return element.best_move._evaluation;
  }

  Bitmove best_move_dummy;
  // Collect the best value from all possible moves
  for (uint8_t i = 0; i < static_cast<uint8_t>(movelist->size()); i++)
  {
    // Copy current board into the preallocated board for this search_ply.
    Bitboard::search_boards[search_ply] = *this;

    // Save history state for current position.
    History_state saved_history_state = history.get_state();

    // Make the selected move on the "ply-board" and ask min() to evaluate it further.
    search_boards[search_ply].make_move(*next_movelist, (*movelist)[i], Gentype::All);
    //merge level_boards[search_ply].make_move((*_movelist)[i], (search_ply < max_search_ply)? Gentype::All:Gentype::Captures);
    //std::cout << search_ply << level_boards[search_ply].last_move() << std::endl;
    move_score = -search_boards[search_ply].negamax_with_pruning(search_ply, -beta, -alpha, best_move_dummy, search_depth);

    // Restore game history and state to current position.
    history.takeback_moves(saved_history_state);
    takeback_from_state(takeback_list[search_ply - 1].state_S);

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
      return beta;
    }
    if (move_score > alpha)
    {
      // Update alpha value.
      // We have found a better move.
      best_move = (*movelist)[i];
      best_move._evaluation = move_score;
      alpha = move_score;
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
  // Save evaluation of the position to the cash and return the best
  // value among the possible moves.
  // ---------------------------------------
  element.best_move = best_move;
  element.best_move._evaluation = alpha;
  element.search_ply = search_ply;
  // ---------------------------------------
  return alpha;
}

} // End namespace C2_chess


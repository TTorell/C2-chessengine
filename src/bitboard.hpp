/*
 * Bitboard.hpp
 *
 *  Created on: Apr 3, 2021
 *      Author: torsten
 */

#ifndef __BITBOARD
#define __BITBOARD

#include <cstdint>
#include <cstring>
#include <cmath>
#include <cassert>
#include <ostream>
#include <sstream>
#include <bit>
#include <bitset>
#include <deque>
#include <atomic>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "zobrist_bitboard_hash.hpp"

namespace C2_chess
{

// constexpr is a way of telling the compiler
// that we wish these expressions to be calculated
// at compile time. For simple constants I don't
// know if it makes any difference compared to
// declaring them with just "const". Isn't normal
// constants instantiated at compile time?
// For more complex expressions and functions it
// has significance though.

constexpr uint64_t a_file = 0x8080808080808080;
constexpr uint64_t b_file = 0x4040404040404040;
constexpr uint64_t c_file = 0x2020202020202020;
constexpr uint64_t d_file = 0x1010101010101010;
constexpr uint64_t e_file = 0x0808080808080808;
constexpr uint64_t f_file = 0x0404040404040404;
constexpr uint64_t g_file = 0x0202020202020202;
constexpr uint64_t h_file = 0x0101010101010101;
constexpr uint64_t row_1 = 0xFF00000000000000;
constexpr uint64_t row_2 = 0x00FF000000000000;
constexpr uint64_t row_3 = 0x0000FF0000000000;
constexpr uint64_t row_4 = 0x000000FF00000000;
constexpr uint64_t row_5 = 0x00000000FF000000;
constexpr uint64_t row_6 = 0x0000000000FF0000;
constexpr uint64_t row_7 = 0x000000000000FF00;
constexpr uint64_t row_8 = 0x00000000000000FF;

constexpr uint64_t zero = 0x0000000000000000;
constexpr uint64_t one = 0x0000000000000001;
constexpr uint64_t whole_board = 0xFFFFFFFFFFFFFFFF;

constexpr uint64_t not_a_file = whole_board ^ a_file;
constexpr uint64_t a_b_files = a_file | b_file;
constexpr uint64_t not_a_b_files = not_a_file ^ b_file;
constexpr uint64_t not_h_file = whole_board ^ h_file;
constexpr uint64_t not_g_h_files = not_h_file ^ g_file;
constexpr uint64_t g_h_files = g_file | h_file;
constexpr uint64_t not_row_8 = whole_board ^ row_8;
constexpr uint64_t not_row_1 = whole_board ^ row_1;

constexpr uint64_t file[] = {a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file};

// 0 is not used as rank-index
constexpr uint64_t rank[] = {zero, row_1, row_2, row_3, row_4, row_5, row_6, row_7, row_8, zero, zero};

constexpr uint64_t king_side = e_file | f_file | g_file | h_file;
constexpr uint64_t queen_side = a_file | b_file | c_file | d_file;
constexpr uint64_t lower_board_half = row_1 | row_2 | row_3 | row_4;
constexpr uint64_t upper_board_half = row_5 | row_6 | row_7 | row_8;

// Squares, some are only used in the unit-test program
// and some may not be used at all.
constexpr uint64_t a1_square = a_file & row_1;
constexpr uint64_t a7_square = a_file & row_7;
constexpr uint64_t a8_square = a_file & row_8;
constexpr uint64_t b1_square = b_file & row_1;
constexpr uint64_t b8_square = b_file & row_8;
constexpr uint64_t c1_square = c_file & row_1;
constexpr uint64_t c2_square = c_file & row_2;
constexpr uint64_t c8_square = c_file & row_8;
constexpr uint64_t c7_square = c_file & row_7;
constexpr uint64_t d1_square = d_file & row_1;
constexpr uint64_t d4_square = d_file & row_4;
constexpr uint64_t d5_square = d_file & row_5;
constexpr uint64_t d7_square = d_file & row_7;
constexpr uint64_t d8_square = d_file & row_8;
constexpr uint64_t e1_square = e_file & row_1;
constexpr uint64_t e2_square = e_file & row_2;
constexpr uint64_t e3_square = e_file & row_3;
constexpr uint64_t e4_square = e_file & row_4;
constexpr uint64_t e5_square = e_file & row_5;
constexpr uint64_t e6_square = e_file & row_6;
constexpr uint64_t e7_square = e_file & row_7;
constexpr uint64_t e8_square = e_file & row_8;
constexpr uint64_t f1_square = f_file & row_1;
constexpr uint64_t f2_square = f_file & row_2;
constexpr uint64_t f7_square = f_file & row_7;
constexpr uint64_t f8_square = f_file & row_8;
constexpr uint64_t g1_square = g_file & row_1;
constexpr uint64_t g2_square = g_file & row_2;
constexpr uint64_t g8_square = g_file & row_8;
constexpr uint64_t h1_square = h_file & row_1;
constexpr uint64_t h8_square = h_file & row_8;

constexpr uint64_t center_squares = d4_square | d5_square | e4_square | e5_square;

constexpr uint64_t rook_initial_squares_white = a1_square | h1_square;
constexpr uint64_t rook_initial_squares_black = a8_square | h8_square;
constexpr uint64_t knight_initial_squares_white = b1_square | g1_square;
constexpr uint64_t knight_initial_squares_black = b8_square | g8_square;
constexpr uint64_t bishop_initial_squares_white = c1_square | f1_square;
constexpr uint64_t bishop_initial_squares_black = c8_square | f8_square;

// Square-patterns which can be shifted to a specific square:
// King-moves, with the king placed at e4
constexpr uint64_t king_pattern = ((d_file | e_file | f_file) & (row_3 | row_4 | row_5)) ^ e4_square;
// Knight-moves, with the knight placed at e4
constexpr uint64_t knight_pattern = ((d_file | f_file) & (row_6 | row_2)) | ((c_file | g_file) & (row_3 | row_5));
constexpr int e4_square_idx = std::countr_zero(e4_square);

constexpr uint64_t castling_empty_squares_K = (row_1 | row_8) & (f_file | g_file);
constexpr uint64_t castling_empty_squares_Q = (row_1 | row_8) & (b_file | c_file | d_file);

// Generate diagonals and anti-diagonals in compile-time
constexpr uint64_t di(int i)
{
  uint8_t fi = a;
  uint8_t r = 0;
  uint64_t val = zero;
  if (i < 8)
  {
    for (fi = a, r = 8 - i; r <= 8; fi++, r++)
      val |= file[fi] & rank[r];
  }
  else
  {
    for (fi = i - 7, r = 1; fi <= h; fi++, r++)
      val |= file[fi] & rank[r];
  }
  return val;
}
constexpr uint64_t diagonal[15] = {di(0), di(1), di(2), di(3), di(4), di(5), di(6), di(7), di(8), di(9), di(10), di(11), di(12), di(13), di(14)};

constexpr uint64_t ad(const int i)
{
  int8_t fi = a; // important: NOT uint8_t because f-- will then become 255, not -1
  uint8_t r = 1;
  uint64_t val = zero;
  if (i < 8)
  {
    for (r = 1, fi = i; fi >= a; fi--, r++)
      val |= file[fi] & rank[r];
  }
  else
  {
    for (fi = h, r = i - 6; r <= 8; fi--, r++)
      val |= file[fi] & rank[r];
  }
  return val;
}
constexpr uint64_t anti_diagonal[15] = {ad(0), ad(1), ad(2), ad(3), ad(4), ad(5), ad(6), ad(7), ad(8), ad(9), ad(10), ad(11), ad(12), ad(13), ad(14)};

inline uint8_t bit_idx(uint64_t square)
{
  assert(std::has_single_bit(square));
  // return std::countr_zero(square); // -std=c++20
  // Seems a tiny bit slower and eventually calls the same builtin after some type checks.
  return __builtin_ctzll(square);
}

inline uint64_t square(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return one << bit_idx;
}

inline uint8_t file_idx(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return 7 - (bit_idx & 7);
}

inline uint8_t file_idx(uint64_t square)
{
  // return 7 - (bit_idx(square) % 8); // is possibly a tiny little bit slower
  return 7 - (bit_idx(square) & 7);
}

inline uint8_t rank_idx(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return 8 - (bit_idx >> 3);
}

inline uint8_t rank_idx(uint64_t square)
{
  // return 8 - (bit_idx(square) % 8); // is possibly a tiny little bit slower
  return 8 - (bit_idx(square) >> 3);
}

inline uint64_t rightmost_square(const uint64_t squares)
{
  if (squares)
  {
    // return (squares & (squares - 1)) ^ (squares); // Is just a little bit slower
    return square(__builtin_ctzll(squares));
  }
  return zero;
}

inline uint64_t leftmost_square(uint64_t squares)
{
  if (squares)
  {
    // return square(63 - std::countl_zero(squares)); -std=c++20
    return square(63 - __builtin_clzl(squares));
  }
  return zero;
}

inline uint64_t popright_square(uint64_t& squares)
{
  if (squares)
  {
    //    The following also works fine:
    //    uint64_t tmp_squares = squares;
    //    squares &= (squares - 1);
    //    return squares ^ tmp_squares;
    uint64_t square = rightmost_square(squares);
    squares &= (squares - 1);
    return square;
  }
  return zero;
}

inline uint64_t popleft_square(uint64_t& squares)
{
  assert(squares);
  uint64_t sq = leftmost_square(squares);
  squares ^= sq;
  return sq;
}
inline uint64_t to_file(uint64_t square)
{
  return file[file_idx(square)];
}

inline uint64_t to_rank(uint64_t square)
{
  return rank[rank_idx(square)];
}

inline uint64_t to_diagonal(uint64_t square)
{
  return diagonal[8 - rank_idx(square) + file_idx(square)];
}

inline uint64_t to_diagonal(uint8_t f_idx, uint8_t r_idx)
{
  assert(f_idx < 8 && r_idx <= 8 && r_idx > 0);
  return diagonal[8 - r_idx + f_idx];
}

inline uint64_t to_anti_diagonal(uint64_t square)
{
  return anti_diagonal[file_idx(square) + rank_idx(square) - 1];
}

inline uint64_t to_anti_diagonal(uint8_t f_idx, uint8_t r_idx)
{
  assert(f_idx < 8 && r_idx <= 8 && r_idx > 0);
  return anti_diagonal[f_idx + r_idx - 1];
}

// Precondition: sq1 and sq2 must be on the same file, rank, diagonal or antidiagonal.
inline uint64_t between(uint64_t sq1, uint64_t sq2, uint64_t squares, bool diagonals = false)
{
  assert(squares);
  uint64_t common_squares;
  if (diagonals)
    common_squares = squares & (to_diagonal(sq2) | to_anti_diagonal(sq2));
  else
    common_squares = squares & (to_file(sq2) | to_rank(sq2));
  if (sq1 > sq2)
    return common_squares & ((sq1 - one) ^ ((sq2 << 1) - one));
  else
    return common_squares & ((sq2 - one) ^ ((sq1 << 1) - one));
}

//inline uint8_t popright_bit_idx(uint64_t& squares)
//{
//  assert(squares);
//  // uint8_t idx = std::countr_zero(squares);
//  uint8_t idx = __builtin_ctzll(squares);
//  squares &= (squares - 1);
//  return idx;
//}

inline uint64_t adjust_pattern(uint64_t pattern, uint64_t center_square)
{
  assert(pattern && std::has_single_bit(center_square));
  uint64_t squares;
  int shift = bit_idx(center_square) - e4_square_idx;
  squares = (shift >= 0) ? (pattern << shift) : (pattern >> -shift);
  // Remove squares in the pattern which may have
  // been shifted over to the other side of the board.
  // (the pattern is max 5 bits wide, so we can remove
  // two files regardless of if center_square is on a-,
  // or b-file etc).
  if (to_file(center_square) & a_b_files)
    squares &= not_g_h_files;
  else if (to_file(center_square) & g_h_files)
    squares &= not_a_b_files;
  return squares;
}

inline uint64_t ortogonal_squares(uint64_t square)
{
  uint8_t b_idx = bit_idx(square);
  return file[file_idx(b_idx)] | rank[rank_idx(b_idx)];
}

inline uint64_t diagonal_squares(uint64_t square)
{
  return to_diagonal(square) | to_anti_diagonal(square);
}

struct BitMove
{
    uint32_t _move;
    float _evaluation;
    BitMove() :
      _move(0),
      _evaluation(0.0)
    {
    }

    BitMove(piecetype p_type,
            uint8_t move_props,
            uint64_t from_square,
            uint64_t to_square,
            piecetype promotion_pt = piecetype::Queen) :
        _move(0),
        _evaluation(0.0)
    {
      _move = (index(p_type) << 24) | (move_props << 14) | (index(promotion_pt) << 12) | (bit_idx(from_square) << 6) | bit_idx(to_square);
    }

    bool operator==(const BitMove& move) const
    {
      if (from() == move.from() && to() == move.to() && piece_type() == move.piece_type())
        return true;
      return false;
    }

    uint64_t to() const
    {
      return square(_move & 0x3F);
    }

    uint64_t from() const
    {
      return square((_move >> 6) & 0x3F);
    }

    piecetype promotion_piece_type() const
    {
      return static_cast<piecetype>((_move >> 12) & 0x03);
    }

    uint8_t properties() const
    {
      return (_move >> 14) & 0xFF;
    }

    piecetype piece_type() const
    {
      return static_cast<piecetype>(_move >> 24);
    }

    void add_property(uint8_t property)
    {
      uint32_t tmp = property;
      _move |= tmp << 14;
    }
    friend std::ostream& operator<<(std::ostream& os, const BitMove& m);
};

struct Bitpieces
{
    uint64_t King;
    uint64_t Queens;
    uint64_t Rooks;
    uint64_t Bishops;
    uint64_t Knights;
    uint64_t Pawns;
    uint64_t pieces;
    void assemble_pieces()
    {
      pieces = King | Queens | Rooks | Bishops | Knights | Pawns;
    }
};

class Bitboard
{
  protected:
    // Static declarations, incomplete type.
    static Zobrist_bitboard_hash transposition_table;
    static Bitboard level_boards[];
    static std::atomic<bool> time_left;

    uint64_t _hash_tag;
    std::deque<BitMove> _movelist;
    col _col_to_move = col::white;
    uint8_t _castling_rights = castling_rights_none;
    bool _has_castled[2];
    uint64_t _ep_square = zero;
    float _material_diff;
    BitMove _last_move;
    uint64_t _checkers;
    uint64_t _pinners;
    uint64_t _pinned_pieces;
    uint64_t _all_pieces;
    Bitpieces _white_pieces;
    Bitpieces _black_pieces;
    Bitpieces* _own;
    Bitpieces* _other;

    // ### Protected basic Bitboard_functions ###
    // ------------------------------------------

    void init_piece_state();

    inline void clear_movelist();

    // ### Protected Methods for move-generation
    // -----------------------------------------

    void find_long_castling();

    void find_short_castling();

    uint64_t find_blockers(uint64_t sq, uint64_t mask, uint64_t all_pieces);

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask);

    void find_Queen_Rook_and_Bishop_moves();

    void find_legal_moves_for_pinned_pieces();

    void find_Knight_moves();

    void find_Pawn_moves();

    void find_normal_legal_moves();

    void find_Knight_moves_to_square(const uint64_t to_square);

    bool check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square);

    void try_adding_ep_pawn_move(uint64_t from_square);

    void add_pawn_move_check_promotion(uint64_t from_square, uint64_t to_square);

    void find_pawn_moves_to_empty_square(uint64_t to_square);

    void find_moves_to_square(uint64_t to_square);

    void find_moves_after_check(uint64_t checker);

    void find_checkers_and_pinned_pieces();

    bool square_is_threatened(uint64_t square, bool King_is_asking) const;

    bool square_is_threatened2(uint64_t to_square, bool King_is_asking);

    inline void find_king_moves();

    // ### Protected methods for making a move ###
    // ---------------------------------------

    inline void add_promotion_piece(piecetype p_type);

    inline void touch_piece(uint64_t from_square);

    inline void remove_other_piece(uint64_t square);

    inline void move_piece(uint64_t from_square,
                           uint64_t to_square,
                           piecetype p_type);

    inline void remove_castling_right(uint8_t cr);

    inline void place_piece(piecetype p_type, uint64_t square);

    inline void clear_ep_square();

    inline void set_ep_square(uint64_t ep_square);

    void update_hash_tag(uint64_t square, col p_color, piecetype p_type);

    inline void update_hash_tag(uint64_t square1, uint64_t square2, col p_color, piecetype type);

    void update_col_to_move();

    inline void update_state_after_king_move(const BitMove& m);

    // ### Protected methods for searching for the best move
    // -----------------------------------------------------

    void count_pawns_in_centre(float& sum, float weight) const;
    void count_castling(float& sum, float weight) const;
    void count_development(float& sum, float weight) const;
    void count_center_control(float& sum, float weight) const;
    int count_threats_to_square(uint64_t square, col color) const;
    float evaluate_position(col for_color, uint8_t level) const;

  public:

    Bitboard();
    Bitboard(const Bitboard& b);

    Bitboard& operator=(const Bitboard& from);

    // Clears thing which could matter when reading a new position
    void clear()
    {
      std::memset(_own, 0, sizeof(Bitpieces));
      std::memset(_other, 0, sizeof(Bitpieces));
      std::memset(_has_castled, 0, 2*sizeof(bool));
      _ep_square = zero;
      _castling_rights = castling_rights_none;
      _material_diff = 0.0;
      _movelist.clear();
    }

    // Reads the position from a text-string, with some
    // syntactic error-checking.
    // Returns -1 if the text-string wasn't accepted.
    int read_position(const std::string& FEN_string);

    // Puts all legal moves of the position in _movelist.
    // (Naturally only the moves for the player who's in
    // turn to make a move.)
    void find_all_legal_moves();

    // ### Public methods for make move ###
    // ------------------------------------

    // Looks up the i:th move in movelist and makes it.
    // move_no just keeps track of the full move number
    // in the game.
    void make_move(uint8_t i, uint8_t& move_no);

    // Only for the command-line interface, where the user
    // is prompted to enter the new move on the keyboard,
    // if he's on turn.
    int make_move(playertype player_type, uint8_t& move_no);

    // This make_move() doesn't require a defined movelist.
    void make_move(const BitMove& m, uint8_t& move_no);

    // All properties of a move are not decided immediately,
    // but some of them (check for instance) are set after the
    // move has been made and the position has been initialized
    // for the other player. So we save the move, until then,
    // in _last_move. These move-properties are mostly for the
    // presentation of the move in text, and unnecessary for
    // other purposes.
    BitMove last_move() const
    {
      return _last_move;
    }

    void set_mate()
    {
      _last_move.add_property(move_props_mate);
    }

    void set_stalemate()
    {
      _last_move.add_property(move_props_stalemate);
    }

    // The chess-engine keeps track of the game, which is
    // not necessary, but nice to see in the log-files.
    // For guessing the opponents last move (from the
    // FEN-string in the "set position" UCI-command) we need
    // to compare two positions and the following methods
    // came in handy.
    uint64_t all_pieces() const
    {
      return _all_pieces;
    }

    Bitpieces* other() const
    {
      return _other;
    }

    // A slow way of determining the piecetype of a piece on
    // given square. Should not be used inside the search-loop.
    piecetype get_piece_type(uint64_t square) const;

    // ### Public methods for the search of best move. ###
    // ---------------------------------------------------

    void set_time_left(bool value);

    static void start_timer(const std::string& max_search_time);

    void start_timer_thread(const std::string& max_search_time);

    bool has_time_left();

    void clear_hash();

    void init_material_evaluation();

    // min() and max() are an attempt to implement the recursive
    // min-max-with-alpha-beta-pruning-algorithm.
    // They can most certainly be improved.
    float max(uint8_t level, uint8_t move_no, float alpha, float beta, int8_t& best_move_index, const uint8_t max_search_level) const;
    float min(uint8_t level, uint8_t move_no, float alpha, float beta, int8_t& best_move_index, const uint8_t max_search_level) const;

    std::ostream& write_piece(std::ostream& os, uint64_t square) const;
    std::ostream& write(std::ostream& os, outputtype wt, col from_perspective) const;
};

} // namespace C2_chess
#endif

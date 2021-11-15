/*
 * Bitboard.hpp
 *
 *  Created on: Apr 3, 2021
 *      Author: torsten
 */

#ifndef __BITBOARD
#define __BITBOARD

#include <cstdint>
#include <cassert>
#include <ostream>
#include <sstream>
#include <bit>
#include <bitset>
#include <deque>
#include <cmath>
#include <atomic>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "position.hpp"
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

constexpr uint64_t file[] = { a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file };

// 0 is not used as rank-index
constexpr uint64_t rank[] = { zero, row_1, row_2, row_3, row_4, row_5, row_6, row_7, row_8, zero, zero };

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
constexpr uint64_t c2_square = c_file & row_2;
constexpr uint64_t c7_square = c_file & row_7;
constexpr uint64_t d1_square = d_file & row_1;
constexpr uint64_t d7_square = d_file & row_7;
constexpr uint64_t d8_square = d_file & row_8;
constexpr uint64_t e1_square = e_file & row_1;
constexpr uint64_t e2_square = e_file & row_2;
constexpr uint64_t e3_square = e_file & row_3;
constexpr uint64_t e4_square = e_file & row_4;
constexpr uint64_t e6_square = e_file & row_6;
constexpr uint64_t e7_square = e_file & row_7;
constexpr uint64_t e8_square = e_file & row_8;
constexpr uint64_t f1_square = f_file & row_1;
constexpr uint64_t f2_square = f_file & row_2;
constexpr uint64_t f7_square = f_file & row_7;
constexpr uint64_t f8_square = f_file & row_8;
constexpr uint64_t g2_square = g_file & row_2;
constexpr uint64_t h1_square = h_file & row_1;
constexpr uint64_t h8_square = h_file & row_8;

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
constexpr uint64_t diagonal[15] = { di(0), di(1), di(2), di(3), di(4), di(5), di(6), di(7), di(8), di(9), di(10), di(11), di(12), di(13), di(14) };

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
constexpr uint64_t anti_diagonal[15] = { ad(0), ad(1), ad(2), ad(3), ad(4), ad(5), ad(6), ad(7), ad(8), ad(9), ad(10), ad(11), ad(12), ad(13), ad(14) };

inline uint8_t bit_idx(uint64_t square)
{
  assert(std::has_single_bit(square));
  //  return std::countr_zero(square);
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
  return 7LL - (bit_idx(square) & 7);
}

inline uint8_t rank_idx(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return 8 - (bit_idx >> 3);
}

inline uint8_t rank_idx(uint64_t square)
{
  return 8 - (bit_idx(square) >> 3);
}

struct BitMove
{
    uint32_t move;
    float evaluation;

    BitMove(piecetype p_type,
            uint8_t move_props,
            uint64_t from_square,
            uint64_t to_square,
            piecetype promotion_pt = piecetype::Queen) :
        move(0),
        evaluation(0.0)
    {
      move = (index(p_type) << 24) + (move_props << 14) + (index(promotion_pt) << 12) + (bit_idx(from_square) << 6) + bit_idx(to_square);
    }

    uint64_t to() const
    {
      return square(move & 0x3F);
    }

    uint64_t from() const
    {
      return square((move >> 6) & 0x3F);
    }

    piecetype promotion_piece_type() const
    {
      return static_cast<piecetype>((move >> 12) & 0x03);
    }

    uint8_t properties() const
    {
      return (move >> 14) & 0xFF;
    }

    piecetype piece_type() const
    {
      return static_cast<piecetype>(move >> 24);
    }
};

<<<<<<< Updated upstream
=======
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
      pieces = King | Queens | Rooks | Bishops | Knights |  Pawns;
    }
};
>>>>>>> Stashed changes

struct Piece_state
{
    uint64_t King;
    uint64_t own_pieces;
    uint64_t Queens;
    uint64_t Rooks;
    uint64_t Bishops;
    uint64_t Knights;
    uint64_t Pawns;
    uint64_t other_pieces;
    uint64_t other_King;
    uint64_t other_Queens;
    uint64_t other_Rooks;
    uint64_t other_Bishops;
    uint64_t other_Knights;
    uint64_t other_Pawns;

    uint64_t all_pieces;
    uint64_t checkers;
    uint64_t pinned_pieces; // Squares of our pinned pieces
    uint64_t pinners; // Squares of other side pinners
};

class Bitboard
{
  protected:
    // Static declarations, incomplete type.
    static Zobrist_bitboard_hash bb_hash_table;
    static Bitboard level_boards[]; // declaration, incomplete type
    static std::atomic<bool> time_left;

    uint64_t _hash_tag;
    std::deque<BitMove> _movelist;
    col _col_to_move = col::white;
    uint8_t _castling_rights = castling_rights_none;
    uint64_t _ep_square = zero;
    float _material_diff;
    Bitpieces _white_pieces;
    Bitpieces _black_pieces;
    Bitpieces* _own;
    Bitpieces* _other;
    Piece_state _s;

    // Basic Bitboard_functions
    // ------------------------

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

    inline uint64_t popright_square(uint64_t &squares)
    {
      if (squares == zero)
        return zero;
      uint64_t tmp_squares = squares;
      squares &= (squares - 1);
      return squares ^ tmp_squares;
    }

    inline uint64_t rightmost_square(const uint64_t squares)
    {
      if (squares == zero)
        return zero;
      //  return square(std::countr_zero(squares));
      return (squares & (squares - 1)) ^ (squares);
    }

    inline uint64_t popleft_square(uint64_t &squares)
    {
      assert(squares);
      uint64_t sq = square(63 - std::countl_zero(squares));
      squares ^= sq;
      return sq;
    }

    inline uint64_t leftmost_square(uint64_t squares)
    {
      if (squares == zero)
        return zero;
      return square(63 - std::countl_zero(squares));
    }

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

    void init_piece_state();

    inline void clear_movelist();

    // Methods for move-generation
    // ------------------------------

    void find_long_castling();

    void find_short_castling();

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

    bool square_is_threatened(uint64_t square, bool King_is_asking);

    inline void find_king_moves();

    // Methods for make move
    // ------------------------

    inline void add_promotion_piece(piecetype p_type);

    inline void touch_piece(uint64_t from_square);

    inline void remove_other_piece(uint64_t square);

    inline void move_piece(uint64_t from_square,
                           uint64_t to_square,
                           piecetype p_type);

    inline void remove_castling_right(uint8_t cr);

    inline void place_piece(piecetype p_type, uint64_t square);

//    inline int ep_file();

    inline void clear_ep_square();

    inline void set_ep_square(uint64_t ep_square);

    inline void update_hash_tag(uint64_t square, col p_color, piecetype type);

    inline void update_hash_tag(uint64_t square1, uint64_t square2, col p_color, piecetype type);

    void update_col_to_move();

    inline void update_state_after_king_move(const BitMove &m);

    inline piecetype get_piece_type(uint64_t square);

    uint64_t find_legal_squares(uint64_t sq, uint64_t mask);

  public:

    Bitboard();

    int read_position(const std::string &FEN_string);

    void find_all_legal_moves();

    void make_move(int i);

};

} // namespace C2_chess
#endif

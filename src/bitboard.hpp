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
// know if it mkes any difference compared to
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

// The elements with value 0 can be useful.
// They represent files and ranks outside
// the chess board. They reduce the control
// necessary to check if a file/rank-index
// is allowed (is inside the board).
constexpr uint64_t file[] = {a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file, zero, zero};

// 0 is not used as rank-index
constexpr uint64_t rank[] = {zero, row_1, row_2, row_3, row_4, row_5, row_6, row_7, row_8, zero, zero};

constexpr uint64_t king_side = e_file | f_file | g_file | h_file;
constexpr uint64_t queen_side = a_file | b_file | c_file | d_file;
constexpr uint64_t lower_board_half = row_1 | row_2 | row_3 | row_4;
constexpr uint64_t upper_board_half = row_5 | row_6 | row_7 | row_8;

// Castling
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

// King-moves, with the king placed att e4
constexpr uint64_t king_pattern = ((d_file | e_file | f_file) & (row_3 | row_4 | row_5)) ^ e4_square;
// Knight-moves, with the knight placed att e4
constexpr uint64_t knight_pattern = ((d_file | f_file) & (row_6 | row_2)) | ((c_file | g_file) & (row_3 | row_5));
constexpr int e4_square_idx = std::countr_zero(e4_square);

constexpr uint64_t castling_empty_squares_K = (row_1 | row_8) & (f_file | g_file);
constexpr uint64_t castling_empty_squares_Q = (row_1 | row_8) & (b_file | c_file | d_file);
constexpr uint64_t threatening_pawn_squares_K = (row_2 | row_7) & king_side;
constexpr uint64_t threatening_pawn_squares_Q = ((row_2 | row_7) & (queen_side)) | (e2_square | e7_square);
constexpr uint64_t threatening_knight_squares_K = ((row_2 | row_7) & (d_file | e_file | h_file))
                                                  | ((row_3 | row_6) & king_side);
constexpr uint64_t threatening_knight_squares_Q = (((row_2 | row_3 | row_6 | row_7) & queen_side)
                                                   ^ (c2_square | c7_square))
                                                  | (e2_square | e7_square | e3_square | e6_square | f2_square | f7_square);
constexpr uint64_t di(int i)
{
  uint8_t fi = a, r = 0;
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
  int8_t fi = a; // important: NOT uint8_t becasue f-- will become 255, not -1
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
  return std::countr_zero(square);
}

inline uint8_t file_idx(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return 7 - (bit_idx & 7);
}

inline uint8_t rank_idx(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return 8 - (bit_idx >> 3);
}

inline uint64_t square(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return one << bit_idx;
}


inline uint8_t file_idx(uint64_t square)
{
  return 7 - (bit_idx(square) & 7);
}

inline uint64_t to_file(uint64_t square)
{
  return file[file_idx(square)];
}

inline uint8_t rank_idx(uint64_t square)
{
  return 8 - (bit_idx(square) >> 3);
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
  if (static_cast<int64_t>(sq1 - sq2) > 0)
    return common_squares & ((sq1 - one) ^ ((sq2 << 1) - one));
  else
    return common_squares & ((sq2 - one) ^ ((sq1 << 1) - one));
}



inline uint8_t popright_bit_idx(uint64_t& squares)
{
  assert(squares);
  uint8_t idx = std::countr_zero(squares);
  squares &= (squares - 1);
  return idx;
}

inline uint64_t popright_square(uint64_t& squares)
{
  assert(squares);
  uint64_t tmp_squares = squares;
  squares &= (squares-1);
  return squares ^ tmp_squares;
}

inline uint64_t adjust_pattern(uint64_t pattern, uint64_t center_square)
{
  assert(pattern);
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

//constexpr uint8_t Direction_north = 0x80;
//constexpr uint8_t Direction_south = 0x40;
//constexpr uint8_t Direction_west = 0x20;
//constexpr uint8_t Direction_east = 0x10;
//constexpr uint8_t Direction_northeast = 0x08;
//constexpr uint8_t Direction_southeast = 0x04;
//constexpr uint8_t Direction_northwest = 0x02;
//constexpr uint8_t Direction_southwest = 0x01;

//light squares      0x55AA55AA55AA55AA
//dark squares       0xAA55AA55AA55AA55

struct BitMove
{
    piecetype _piece_type;
    uint8_t _properties;
    piecetype _promotion_piece_type;
    uint64_t _from_square;
    uint64_t _to_square;
    float _evaluation;

    BitMove() :
        _piece_type(piecetype::Pawn),
        _properties(0),
        _promotion_piece_type(piecetype::Queen),
        _from_square(zero),
        _to_square(zero),
        _evaluation(0.0)
    {
      // cout << "BitMove::default_ctor" << endl;
    }

    BitMove(const BitMove& m) :
        _piece_type(m._piece_type),
        _properties(m._properties),
        _promotion_piece_type(m._promotion_piece_type),
        _from_square(m._from_square),
        _to_square(m._to_square),
        _evaluation(m._evaluation)
    {
      // cout << "BitMove::copy_ctor" << endl;
    }

    BitMove(const piecetype type,
            uint8_t props,
            uint64_t from_square,
            uint64_t to_square,
            piecetype ppt = piecetype::Queen) :
        _piece_type(type),
        _properties(props),
        _promotion_piece_type(ppt),
        _from_square(from_square),
        _to_square(to_square),
        _evaluation(0.0)
    {
      //cout << "BitMove::value_ctor: " << *this << endl;
    }

    uint8_t from_f_index() const
    {
      return (7 - (static_cast<int>(LOG2(_from_square))) % 8);
    }
    uint8_t from_r_index() const
    {
      return (8 - (static_cast<int>(LOG2(_from_square))) / 8);
    }
    uint8_t to_f_index() const
    {
      return (7 - (static_cast<int>(LOG2(_to_square))) % 8);
    }
    uint8_t to_r_index() const
    {
      return (8 - (static_cast<int>(LOG2(_to_square))) / 8);
    }

    friend std::ostream& operator <<(std::ostream& os, const BitMove& m);
};

struct Piece_state
{
    uint64_t King;
    uint8_t King_file_index;
    uint8_t King_rank_index;
    uint64_t King_file;
    uint64_t King_diagonal;
    uint64_t King_anti_diagonal;
    uint64_t King_rank;
    uint64_t King_initial_square;
    uint8_t castling_rights_K;
    uint8_t castling_rights_Q;
    uint64_t Rook_initial_square_K;
    uint64_t Rook_initial_square_Q;
    uint64_t own_pieces;
    uint64_t adjacent_files; // Files adjacent to King_square.
    uint64_t r1; // Rank from which white pawns can attack our King.
    uint64_t r2;
    uint64_t adjacent_ranks; // Ranks adjacent to our King_square.
    uint64_t empty_squares;
    uint64_t Queens;
    uint64_t Rooks;
    uint64_t Bishops;
    uint64_t Knights;
    uint64_t Pawns;
    int8_t pawnstep;

    uint64_t other_pieces;
    uint64_t other_King;
    uint64_t other_Queens;
    uint64_t other_Queens_or_Rooks;
    uint64_t other_Queens_or_Bishops;
    uint64_t other_King_Queens_or_Bishops;
    uint64_t other_King_Queens_or_Rooks;
    uint64_t other_Rooks;
    uint64_t other_Bishops;
    uint64_t other_Knights;
    uint64_t other_Pawns;

    uint64_t all_pieces;
    uint64_t checkers;
    uint64_t pinned_pieces; // Squares of our pinned pieces
    uint64_t pinning_pieces; // Squares of other side pinners
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
    Piece_state _s;
    uint64_t _W_King;
    uint64_t _W_Queens;
    uint64_t _W_Rooks;
    uint64_t _W_Bishops;
    uint64_t _W_Knights;
    uint64_t _W_Pawns;
    uint64_t _B_King;
    uint64_t _B_Queens;
    uint64_t _B_Rooks;
    uint64_t _B_Bishops;
    uint64_t _B_Knights;
    uint64_t _B_Pawns;

    uint64_t _W_King_file;
    uint64_t _W_King_rank;
    uint64_t _B_King_file;
    uint64_t _B_King_rank;

    uint64_t _W_King_diagonal;
    uint64_t _W_King_anti_diagonal;
    uint64_t _B_King_diagonal;
    uint64_t _B_King_anti_diagonal;
    int8_t _W_King_file_index;
    int8_t _W_King_rank_index;
    int8_t _B_King_file_index;
    int8_t _B_King_rank_index;


    void init_piece_state();

    inline void clear_movelist();

    inline void find_king_moves();

    bool find_check_or_pinned_piece(uint64_t square,
                                    uint64_t threatening_pieces,
                                    uint64_t opponents_other_pieces,
                                    uint64_t& pinned_piece);

    bool square_is_threatened_old(int8_t file_index,
                                  int8_t rank_index,
                                  bool King_is_asking);

    bool square_is_threatened(uint64_t square, bool King_is_asking);

    inline void contains_checking_piece(const uint64_t square,
                                        const uint64_t pieces,
                                        const uint64_t forbidden_files);

    void find_checkers_and_pinned_pieces();

    void look_for_checks_and_pinned_pieces();

    void step_from_King_to_pinning_piece(uint64_t from_square, uint8_t inc, piecetype p_type, uint64_t pinning_squares);

    void find_legal_moves_for_pinned_pieces();

    inline void try_adding_move(uint64_t pieces,
                                piecetype p_type,
                                uint8_t move_props,
                                uint64_t from_square,
                                uint64_t to_square);

    bool check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square);
    bool check_if_other_pawn_is_pinned_ep(uint64_t other_pawn_square, uint64_t own_pawn_square, uint8_t inc);

    void try_adding_ep_pawn_move(uint64_t from_square);

    void add_pawn_move_check_promotion(uint64_t from_square, uint64_t to_square);

    uint64_t add_two_knight_moves(uint64_t from_square, uint64_t allowed_squares, int8_t inc1, int8_t inc2);

    void find_Pawn_moves();

    void find_Knight_moves();

    void find_Knight_moves_to_square(const uint64_t to_square);

    void find_Bishop_or_Queen_moves();

    inline uint64_t between_extended(uint64_t sq1,
                                     uint64_t sq2,
                                     uint64_t squares,
                                     bool diagonals = false);

    void find_Rook_or_Queen_moves();

    void find_short_castling();

    void find_long_castling();

    bool castling_squares_are_threatened_K();

    bool castling_squares_are_threatened_Q();

    inline uint64_t step_out_from_square(uint64_t square,
                                         int8_t inc,
                                         piecetype& p_type,
                                         uint64_t& pieces);

    void find_pawn_moves_to_empty_square(uint64_t to_square);

    void find_moves_to_square(uint64_t to_square);

    void step_from_King_to_checking_piece(uint8_t inc);

    void find_moves_after_check_old();

    void find_moves_after_check(uint64_t checker);

    void find_normal_legal_moves();

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

    inline void update_state_after_king_move(const BitMove& m);

    inline piecetype get_piece_type(uint64_t square);

  public:

    Bitboard();

    int read_position(const std::string& FEN_string);

    void find_all_legal_moves();

    void make_move(int i);
};

} // namespace C2_chess
#endif

#ifndef _CHESSTYPES
#define _CHESSTYPES

#include <cstdint>
#include <bit>
#include <cassert>
#include <ostream>
#include <limits>
#include <deque>

using namespace std::string_literals;

namespace C2_chess
{

// template for enum class
// So the enum-value can be used as an index
// in arrays, for instance. Didn't want to write
// static_cast<int> everywhere.
template<typename T>
inline int index(const T& val)
{
  return static_cast<int>(val);
}

const auto start_position_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"s; // std::string

const auto infinity = std::numeric_limits<float>::infinity(); // float

const auto a = 0; // int
const auto b = 1;
const auto c = 2;
const auto d = 3;
const auto e = 4;
const auto f = 5;
const auto g = 6;
const auto h = 7;
const auto eval_max = 100.0F;
const auto eval_min = -eval_max;
const auto epsilon = 0.00000001F;

const auto N_SEARCH_BOARDS_DEFAULT = 38U;

const auto dont_update_history = false;
const auto init_pieces = true;
const auto dont_evaluate_zero_moves = false;
const auto use_max_search_depth = true;
const auto on_same_line = true;
const auto on_separate_lines = false;
const auto xray_threats_through_king_allowed = true;

enum class Piecetype
{
  Queen,
  Rook,
  Bishop,
  Knight,
  Pawn,
  King,
  Undefined
};

const float piece_values[7] = {9.0F, 5.0F, 3.0F, 3.0F, 1.0F, 0.0F, 0.0F};

const auto pawn_value = piece_values[index(Piecetype::Pawn)];
const auto queen_value = piece_values[index(Piecetype::Queen)];
const auto rook_value = piece_values[index(Piecetype::Rook)];
const auto knight_value = piece_values[index(Piecetype::Knight)];
const auto bishop_value = piece_values[index(Piecetype::Bishop)];
const auto king_value = piece_values[index(Piecetype::King)];

enum class Color
{
  White = 0,
  Black = 1
};

enum class Playertype
{
  Computer,
  Human
};

enum class Gentype
{
  All,
  Captures,
  Captures_and_Promotions
};

// Castling rights
constexpr uint8_t castling_rights_all = 0B00001111;
constexpr uint8_t castling_rights_none = 0B00000000;

constexpr uint8_t castling_right_WK = 0B00001000;
constexpr uint8_t castling_right_WQ = 0B00000100;
constexpr uint8_t castling_rights_W = 0B00001100;

constexpr uint8_t castling_right_BK = 0B00000010;
constexpr uint8_t castling_right_BQ = 0B00000001;
constexpr uint8_t castling_rights_B = 0B00000011;

// BitMove properties
// Note: only the ten least significant bytes may be used.
constexpr uint16_t move_props_none = 0x0000;
constexpr uint16_t move_props_en_passant = 0x0001;
constexpr uint16_t move_props_promotion = 0x0002;
constexpr uint16_t move_props_capture = 0x0004;
constexpr uint16_t move_props_check = 0x0008;
constexpr uint16_t move_props_mate = 0x0010;
constexpr uint16_t move_props_stalemate = 0x0020;
constexpr uint16_t move_props_castling = 0x0040;
constexpr uint16_t move_props_draw_by_repetition = 0x0080;
constexpr uint16_t move_props_draw_by_50_moves = 0x0100;

struct Search_info
{
    Color searching_side;
    unsigned int leaf_node_counter;
    unsigned int node_counter;
    unsigned int hash_hits;
    unsigned int first_beta_cutoffs;
    unsigned int beta_cutoffs;
    unsigned long time_taken;
    unsigned long max_search_depth;
    bool search_interrupted;
    float score;
    unsigned int highest_search_ply;

    float get_score() const
    {
      return (searching_side == Color::White) ? score : -score;
    }

    friend std::ostream& operator<<(std::ostream& os, const Search_info& si);
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

// The following two functions are needed by Bitmove
inline uint8_t bit_idx(uint64_t square)
{
  assert(std::has_single_bit(square));
  // return std::countr_zero(square); // -std=c++20
  // Seems a tiny bit slower and eventually calls the same builtin after some type checks.
  return __builtin_ctzll(square);
}

constexpr uint64_t zero = 0x0000000000000000;
constexpr uint64_t one = 0x0000000000000001;

inline uint64_t square(uint8_t bit_idx)
{
  assert(bit_idx < 64);
  return one << bit_idx;
}

const uint32_t last_none_valid_move_constant = 4;

struct Bitmove
{
    uint32_t _move;
    float _evaluation;

    Bitmove() :
        _move(0),
        _evaluation(0.0)
    {
    }

    explicit Bitmove(uint32_t move) :
        _move(move),
        _evaluation(0.0)
    {
    }

    Bitmove(Piecetype p_type, // bit 25-32
            uint16_t move_props, // bit 15-24
            uint64_t from_square, // bit 7-12
            uint64_t to_square, // bit 1-6
            Piecetype promotion_pt = Piecetype::Queen) : // bit 13-14
        _move(0),
        _evaluation(0.0)
    {
      _move = (static_cast<uint32_t>(index(p_type)) << 24) | (move_props << 14) | static_cast<uint32_t>(index(promotion_pt) << 12)
              | static_cast<uint32_t>(bit_idx(from_square)) << 6
              | static_cast<uint32_t>(bit_idx(to_square));
    }

    bool operator==(const Bitmove& m) const
    {
      if (_move == m._move)
        return true;
      return false;
    }

    bool operator <(const Bitmove& m) const
    {
      // Sort in descending order
      return m._evaluation < _evaluation;
    }

    uint64_t to() const
    {
      return square(_move & 0x3F);
    }

    uint64_t from() const
    {
      return square((_move >> 6) & 0x3F);
    }

    Piecetype promotion_piece_type() const
    {
      return static_cast<Piecetype>((_move >> 12) & 0x03);
    }

    uint16_t properties() const
    {
      return (_move >> 14) & 0x03FF;
    }

    Piecetype piece_type() const
    {
      return static_cast<Piecetype>(_move >> 24);
    }

    void add_property(uint16_t property)
    {
      uint32_t tmp = property;
      _move |= tmp << 14;
    }

    void evaluation(float val)
    {
      _evaluation = val;
    }

    float evaluation() const
    {
      return _evaluation;
    }

    friend std::ostream& operator<<(std::ostream& os, const Bitmove& m);

    bool is_valid()
    {
      return _move > last_none_valid_move_constant; // Bigger than DRAW_BY_50_MOVES_RULE._move.
    }
};

// Movelist types:
using list_ptr = std::deque<Bitmove>*;
using list_ref = std::deque<Bitmove>&;
using list_t = std::deque<Bitmove>;

struct Takeback_state
{
    list_ptr _movelist = new list_t{};
    uint64_t _hash_tag;
    uint8_t _castling_rights;
    uint8_t _half_move_counter;
    Color _side_to_move = Color::White;
    uint16_t _move_number;
    bool _has_castled_0;
    bool _has_castled_1;
    uint64_t _ep_square = zero;
    float _material_diff;
    Bitmove _last_move;
    Piecetype _taken_piece;
};

struct Takeback_element
{
    Takeback_state state_S; // For Negamax search;
    Takeback_state state_Q; // For Quiescence search;
    friend std::ostream& operator<<(std::ostream& os, const Takeback_element& element);
};

// The returned best_move from a search can contain a valid move of course,
// but it can also contain the following information.
const Bitmove NO_MOVE(0);
// I also use a Bitmove to decide if a TT-element has been initialized,
// or if it's a new empty element returned from the Transposition_table.find(hash_tag) method.
const Bitmove UNDEFINED_MOVE(1);
const Bitmove DRAW_BY_THREEFOLD_REPETITION(2);
const Bitmove DRAW_BY_50_MOVES_RULE(3);
const Bitmove SEARCH_HAS_BEEN_INTERRUPTED(last_none_valid_move_constant); // currently 4

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
constexpr uint64_t d2_square = d_file & row_2;
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

constexpr uint64_t king_initial_squares = e1_square | e8_square;
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
  auto fi = a; // important: NOT uint8_t because f-- may then become 255, not -1
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

} // namespace C2_chess
#endif

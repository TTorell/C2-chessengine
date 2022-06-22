#ifndef _CHESSTYPES
#define _CHESSTYPES

#include <cstdint>

namespace C2_chess
{

const int a = 0;
const int b = 1;
const int c = 2;
const int d = 3;
const int e = 4;
const int f = 5;
const int g = 6;
const int h = 7;
const float eval_max = 100.0F;
const float eval_min = -eval_max;
const float epsilon = 0.00000001F;

const int PV_TABLE_SIZE_DEFAULT = 50000;
const int N_SEARCH_BOARDS_DEFAULT = 38;

enum class piecetype {
  Queen,
  Rook,
  Bishop,
  Knight,
  Pawn,
  King,
  Undefined
};

enum class col {
  white = 0,
  black = 1
};

enum class playertype {
  computer,
  human
};

enum class gentype {
    all,
    captures
};

enum class outputtype {
  verbose,
  silent,
  debug,
  cmd_line_diagram
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
constexpr bool SAME_LINE = true;

} // namespace
#endif

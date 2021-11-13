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
constexpr uint8_t move_props_none = 0x00;
constexpr uint8_t move_props_en_passant = 0x01;
constexpr uint8_t move_props_promotion = 0x02;
constexpr uint8_t move_props_castling = 0x40;
constexpr uint8_t move_props_capture = 0x04;
constexpr uint8_t move_props_check = 0x08;
constexpr uint8_t move_props_mate = 0x10;
constexpr uint8_t move_props_stalemate = 0x20;

} // namespace
#endif

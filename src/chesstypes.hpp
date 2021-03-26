#ifndef _CHESSTYPES
#define _CHESSTYPES

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

// template for enum class
// So the enum-value can be used as an int-index
// in arrays, for instance. Didn't want to write
// static_cast<int> everywhere.
template<typename T>
inline int index(const T& val)
{
  return static_cast<int>(val);
}

enum class piecetype {
  King,
  Queen,
  Rook,
  Bishop,
  Knight,
  Pawn,
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
} // namespace
#endif

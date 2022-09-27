/*
 * chessfuncs.hpp
 *
 *  Created on: 15 feb. 2019
 *      Author: torsten
 *
 *  Contains declaration of some useful functions
 */

#ifndef CHESSFUNCS_HPP_
#define CHESSFUNCS_HPP_

#include <cassert>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
//#include <utility>
#include <bit>
#include <bitset>
#include <cmath>

#include "chesstypes.hpp"

namespace fs = std::filesystem;

// Two templates I copied from internet to check
// the types of auto-defined variables.

#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <memory>
#include <cstdlib>

template <class T>
std::string
type_name()
{
    typedef typename std::remove_reference<T>::type TR;
    std::unique_ptr<char, void(*)(void*)> own
           (
#ifndef _MSC_VER
                abi::__cxa_demangle(typeid(TR).name(), nullptr,
                                           nullptr, nullptr),
#else
                nullptr,
#endif
                std::free
           );
    std::string r = own != nullptr ? own.get() : typeid(TR).name();
    if (std::is_const<TR>::value)
        r += " const";
    if (std::is_volatile<TR>::value)
        r += " volatile";
    if (std::is_lvalue_reference<T>::value)
        r += "&";
    else if (std::is_rvalue_reference<T>::value)
        r += "&&";
    return r;
}

#include <string_view>

template <typename T>
constexpr auto type_name2() {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto type_name() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto type_name() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl type_name<";
  suffix = ">(void)";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}

// templates for comparing floats or doubles
template<typename T>
inline bool is_close(T val1, T val2, T marginal)
{
  if (fabs(val1 - val2) <= marginal)
    return true;
  return false;
}

template<typename T>
inline bool is_close(T val1, T val2)
{
  const T marginal = 1e-10;
  return is_close(val1, val2, marginal);
}

template<typename T>
inline bool is_in_vector(const std::vector<T>& v, const T& element)
{
  for (const T& e:v)
  {
    if (e == element)
      return true;
  }
  return false;
}

// A fast log2 function for integer types.
// Uses a gcc-function
#ifdef __linux__
template<typename T>
inline unsigned int LOG2(const T& val)
{
  return (64 - __builtin_clzl((static_cast<long unsigned int>(val))) - 1);
}
#endif

template<typename T>
std::ostream& write_list(const std::deque<T>& list, std::ostream& os, bool same_line = false)
{
  bool first = true;
  for (const T& element : list)
  {
    if (!first && same_line)
      os << " ";
    os << element;
    if (!same_line)
      os << std::endl;
    first = false;
  }
  os << std::endl;
  return os;
}

template<typename T>
std::ostream& write_vector(const std::vector<T>& list, std::ostream& os, bool same_line = false)
{
  bool first = true;
  for (const T& element : list)
  {
    if (!first && same_line)
      os << " ";
    os << element;
    if (!same_line)
      os << std::endl;
    first = false;
  }
  os << std::endl;
  return os;
}

template<typename T>
T size_align(T base, T number)
{
  static_assert( std::is_integral<T>());
  if (number % base != 0)
    return (number / base + 1) * base;
  else
    return number;
}

namespace C2_chess
{

template<typename T>
std::string to_binary(const T& x)
{
  std::stringstream ss;
  ss << std::bitset<sizeof(T) * 8>(x);
  return ss.str();
}

bool is_positive_number(const std::string& s);

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

class Config_params;

Color other_color(const Color& side);
inline Color& operator++(Color& side);
Color col_from_string(const std::string& s);
std::string get_logfile_name();
void require(bool b, std::string file, std::string method, int line);
void require_m(bool b, std::string file, std::string method, int line, const Bitmove& m);
std::ostream& print_backtrace(std::ostream& os);
std::string get_stdout_from_cmd(std::string cmd);
std::pair<std::string, int> exec(const char* cmd);
bool check_execution_dir(const std::string& preferred_exec_dir);
bool regexp_match(const std::string& line, const std::string& regexp_string);
bool regexp_grep(const std::string& line, const std::string& regexp_string);
bool regexp_grep(const std::string& line, const std::string& regexp_string, std::vector<std::string>& matches);
std::string rexexp_sed(const std::string& line, const std::string& regexp_string, const std::string& replacement_string);
std::vector<std::string> split(const std::string& s, char delim);
std::vector<std::string> split(const std::string& input, std::string& delimiter);
std::string cut(const std::string& s, char delim, uint64_t field_number);
void play_on_cmd_line(Config_params& config_params);
void print_filetype(std::ostream& os, const fs::file_status& s);
void print_filepermissions(fs::perms p);
bool is_regular_read_OK(const fs::path& filepath);
bool is_regular_write_OK(const fs::path& filepath);
bool has_duplicates(const std::vector<std::string>& vector, const std::string& move_kind);
bool compare_move_lists(const std::vector<std::string>& out_vector, const std::vector<std::string>& ref_vector);
bool all_in_a_exist_in_b(const std::vector<std::string>& a, const std::vector<std::string>& b, bool order_out_ref);
std::string to_binary_board(uint64_t in);
bool question_to_user(const std::string& question, std::string regexp_correct_answer);
std::string user_input(const std::string& message);
std::string reverse_FEN_string(const std::string& FEN_string);
std::vector<std::string> reverse_moves(const std::vector<std::string>& moves);
} // End namespace C2_chess
#endif //CHESSFUNCS_HPP_

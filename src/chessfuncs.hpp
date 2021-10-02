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

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
//#include <utility>
#include <bitset>
#include "chesstypes.hpp"

namespace fs = std::filesystem;

// template for enum class
// So the enum-value can be used as an int-index
// in arrays, for instance. Didn't want to write
// static_cast<int> everywhere.
template<typename T>
inline int index(const T& val)
{
  return static_cast<int>(val);
}

// A fast log2 function for integer types.
// Uses a gcc-functio
#ifdef __linux__
template<typename T>
inline unsigned int LOG2(const T& val)
{
  return (64 - __builtin_clzl((static_cast<long unsigned int>(val))) - 1);
}
#endif

namespace C2_chess
{

template<typename T>
std::string to_binary(const T& x)
{
  std::stringstream ss;
  ss << std::bitset<sizeof(T) * 8>(x);
  return ss.str();
}

class Move;
class Config_params;

col other_color(const col& c);
inline col& operator++(col& c);
col col_from_string(const std::string& s);
std::string get_logfile_name();
void require(bool b, std::string file, std::string method, int line);
void require_m(bool b, std::string file, std::string method, int line, const Move &m);
std::ostream& print_backtrace(std::ostream &os);
std::string get_stdout_from_cmd(std::string cmd);
std::pair<std::string, int> exec(const char* cmd);
bool check_execution_dir(const std::string& preferred_exec_dir);
bool regexp_match(const std::string &line, const std::string &regexp_string);
bool regexp_grep(const std::string &line, const std::string &regexp_string);
bool regexp_grep(const std::string& line, const std::string& regexp_string, std::vector<std::string>& matches);
std::string rexexp_sed(const std::string& line, const std::string& regexp_string, const std::string& replacement_string);
std::vector<std::string> split(const std::string& s, char delim);
std::vector<std::string> split(const std::string& input, std::string& delimiter);
std::string cut(const std::string& s, char delim, int field_number);
std::string iso_8859_1_to_utf8(const std::string& str);
std::string iso_8859_1_to_utf8(const char *c_string);
void play_on_cmd_line(Config_params& config_params);
void print_filetype(std::ostream& os, fs::file_status& s);
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

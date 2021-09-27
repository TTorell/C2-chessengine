/*
 * chessfuncs.cpp
 *
 *  Created on: 15 feb. 2019
 *      Author: Torsten
 *
 *  Contains some useful functions
 */
#include <filesystem>
#include <iostream>
#include <sstream>
//#include <iomanip>
#include <regex>
//#include <memory>
//#include <array>
//#include <utility>
//#include <cstdlib>
//#include <cstring>
//#include <cstdio>
//#include <ctime>
//#include <bitset>
//#include <sys/stat.h>
#include "chesstypes.hpp"
#include "chessfuncs.hpp"
#include "move.hpp"

// Redefining a namespace to something shorter.
namespace fs = std::filesystem;


#ifdef __linux__
extern "C"
{
#include <sys/wait.h>
#include <unistd.h>
}
#endif // __linux__

namespace C2_chess
{

col other_color(col& c)
{
  return (c == col::white) ? col::black : col::white;
}

col& operator++(col& c)
{
  return c = (c == col::white) ? col::black : col::white;
}

col col_from_string(const std::string& s)
{
  if (s == "w")
    return col::white;
  else
    return col::black;
}

std::string get_logfile_name()
{
  auto time = std::time(nullptr);
  std::stringstream ss;
  ss << "C2_log_" << std::put_time(localtime(&time), "%F-%T") << ".txt"; // ISO 8601 format.
  return ss.str();
}

#ifdef __linux__
std::ostream& print_backtrace(std::ostream& os)
{
  char pid_buf[30];
  sprintf(pid_buf, "--pid=%d", getpid());
  char name_buf[512];
  name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
  int child_pid = fork();
  if (!child_pid)
  {
    dup2(2, 1); // redirect output to stderr
    os << "stack trace for " << name_buf << " pid=" << pid_buf << std::endl;
    execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
    abort(); /* If gdb failed to start */
  }
  else
  {
    waitpid(child_pid, NULL, 0);
  }
  return os;
}
#endif // linux

void require(bool bo, std::string file, std::string method, int line)
{
  if (!bo)
  {
    std::cerr << "Requirement error in " << file << ":" << method << " at line " << line << std::endl;
#ifdef __linux
    print_backtrace(std::cerr) << std::endl;
#endif // linux

    exit(-1);
  }
}

void require_m(bool bo, std::string file, std::string method, int line, const Move &m)
{
  if (!bo)
  {
    std::cerr << "Move:: requirement error in " << file << ":" << method << " at line " << line << std::endl;
    std::cerr << "Move = " << m << std::endl << std::endl;
#ifdef __linux__
    print_backtrace(std::cerr) << std::endl;
#endif // linux

    exit(-1);
  }
}

#ifdef __linux
#define open_pipe popen
#define close_pipe pclose
#else
#define open_pipe _popen
#define close_pipe _pclose
#endif

// Run a command and return it's output as a string.
// This function returns the output (stdout and stderr) from a command in the environment
// you run the program from.
// NOTE: Also the trailing newline from the command will be present in the result.
std::string get_stdout_from_cmd(std::string cmd)
{

  std::string data;
  FILE* stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  cmd.append(" 2>&1"); // redirect stderr to same as stdin

  stream = open_pipe(cmd.c_str(), "r");
  if (stream)
  {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL)
        data.append(buffer);
    close_pipe(stream);
  }
  return data;
}

// This function is almost the same as get_stdout_from_cmd(), but it also gives the return
// value from the command.
// NOTE: Also the trailing newline from the command output will be present in the result string.
std::pair<std::string, int> exec(const char* cmd)
{
  std::array<char, 128> buffer;
  std::string result;
  int return_code = -1;
  auto pclose_wrapper = [&return_code](FILE* cmd)
                                       {
                                         return_code = close_pipe(cmd);
                                       };
  {
    // scope is important, have to make sure the ptr goes out of scope first
    const std::unique_ptr<FILE, decltype(pclose_wrapper)> pipe(open_pipe(cmd, "r"), pclose_wrapper);
    if (pipe)
    {
      while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      {
        result += buffer.data();
      }
    }
  }
  return make_pair(result, return_code);
}

bool check_execution_dir(const std::string& preferred_exec_dir)
{
  const std::pair<std::string, int> process_return = exec("pwd 2>&1");
  if (process_return.second != 0)
  {
    std::cerr << "ERROR: could not execute command \"pwd\"" << std::endl
         << "program exited with status code "
         << process_return.second << std::endl
         << "captured stdout and stderr; "
         << std::endl
         << process_return.first
         << std::endl;
    return false;
  }
  std::string current_dir = process_return.first;
  // Remove trailing newline.
  current_dir = current_dir.substr(0, current_dir.length() - 1);
  if (current_dir != preferred_exec_dir)
  {
    std::cerr << "ERROR: You should run this program from:" << std::endl << preferred_exec_dir << std::endl;
    std::cerr << "You started it from: " << std::endl << current_dir << std::endl;
    return false;
  }
  return true;
}

// Here the regexp_string (pattern) must match the whole line.
bool regexp_match(const std::string& line, const std::string& regexp_string)
{
  std::smatch m;
  std::regex r(regexp_string);
  return (std::regex_match(line, m, r));
}

// Tries to find a part of line which matches the regexp_string.
// similar to the grep-function in Unix shells.
bool regexp_grep(const std::string& line, const std::string& regexp_string)
{
  std::smatch m;
  std::regex r(regexp_string);
  return (std::regex_search(line, m, r));
}

// Tries to find a part of line which matches the regexp_string.
// similar to the grep-function in Unix shells.
// The matching string(s) in line are put in the string-vector called matches.
bool regexp_grep(const std::string& line, const std::string& regexp_string, std::vector<std::string>& matches)
{
  std::smatch m;
  std::regex r(regexp_string);
  bool found = (std::regex_search(line, m, r));
  for (const std::string& s : m)
    matches.push_back(s);
  return found;
}

std::string rexexp_sed(const std::string& line, const std::string& regexp_string, const std::string& replacement_string)
{
  // regex_replace example:
  //#include <sstreamream>
  //#include <string>
  //#include <regex>
  //#include <iterator>
  //
  //  int main ()
  //  {
  //    std::string s ("there is a subsequence in the string\n");
  //    std::regex e ("\\b(sub)([^ ]*)");   // matches words beginning by "sub"
  //
  //    // using string/c-string (3) version:
  //    std::cout << std::regex_replace (s,e,"sub-$2");
  //
  //    // using range/c-string (6) version:
  //    std::string result;
  //    std::regex_replace (std::back_inserter(result), s.begin(), s.end(), e, "$2");
  //    std::cout << result;
  //
  //    // with flags:
  //    std::cout << std::regex_replace (s,e,"$1 and $2",std::regex_constants::format_no_copy);
  //    std::cout << std::std::endl;
  //
  //    return 0;
  //  }
  //
  //
  //  Edit & Run
  //
  //
  //  Output:
  //
  //  there is a sub-sequence in the string
  //  there is a sequence in the string
  //  sub and sequence

  std::regex re(regexp_string);
  return std::regex_replace(line, re, replacement_string);
}

std::vector<std::string> split(const std::string& s, char delim)
{
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;

  while (getline(ss, item, delim))
  {
    result.push_back(item);
  }

  return result;
}

std::string cut(const std::string& s, char delim, int field_number)
{
  std::vector<std::string> tokens = split(s, delim);
  return tokens[field_number - 1];
}

// Split where delimiter consists of multiple chars
std::vector<std::string> split(const std::string& input, std::string& delimiter)
{
  std::istringstream ss(input);
  std::string token;
  std::string::iterator it;
  std::vector<std::string> tokens;
  while (std::getline(ss, token, *(it = delimiter.begin())))
  {
    while (*(++it))
      ss.get();
    tokens.push_back(token);
  }
  return tokens;
}

std::string iso_8859_1_to_utf8(const std::string& str)
{
  std::string str_out;
  for (uint8_t ch : str)
  {
    if (ch < 0x80)
    {
      str_out.push_back(ch);
    }
    else
    {
      str_out.push_back(0xc0 | ch >> 6);
      str_out.push_back(0x80 | (ch & 0x3f));
    }
  }
  return str_out;
}

std::string iso_8859_1_to_utf8(const char* c_string)
{
  std::string str_out(c_string);
  return iso_8859_1_to_utf8(str_out);
}

void print_filetype(std::ostream& os, fs::file_status s)
{
  os << "It";
  // alternative: switch(s.type()) { case fs::file_type::regular: ...}
  if (fs::is_regular_file(s))
    os << " is a regular file.\n";
  if (fs::is_directory(s))
    os << " is a directory.\n";
  if (fs::is_block_file(s))
    os << " is a block device.\n";
  if (fs::is_character_file(s))
    os << " is a character device.\n";
  if (fs::is_fifo(s))
    os << " is a named IPC pipe.\n";
  if (fs::is_socket(s))
    os << " is a named IPC socket.\n";
  if (fs::is_symlink(s))
    os << " is a symlink.\n";
  if (!fs::exists(s))
    os << " does not exist\n";
}

void print_filepermissions(fs::perms p)
{
  std::cerr << ((p & fs::perms::owner_read) != fs::perms::none ? "r" : "-")
            << ((p & fs::perms::owner_write) != fs::perms::none ? "w" : "-")
            << ((p & fs::perms::owner_exec) != fs::perms::none ? "x" : "-")
            << ((p & fs::perms::group_read) != fs::perms::none ? "r" : "-")
            << ((p & fs::perms::group_write) != fs::perms::none ? "w" : "-")
            << ((p & fs::perms::group_exec) != fs::perms::none ? "x" : "-")
            << ((p & fs::perms::others_read) != fs::perms::none ? "r" : "-")
            << ((p & fs::perms::others_write) != fs::perms::none ? "w" : "-")
            << ((p & fs::perms::others_exec) != fs::perms::none ? "x" : "-")
            << '\n';
}

bool is_regular_read_OK(const fs::path& filepath)
{
  // print_filepermissions(fs::status(filepath).permissions());
  if (!std::filesystem::exists(filepath))
  {
    std::cout << "Error: The file " << filepath << " doesn't exist." << std::endl;
    return false;
  }
  if (!std::filesystem::is_regular_file(filepath))
  {
    std::cout << "Error: " << filepath << " is not a regular file." << std::endl;
    print_filetype(std::cout, fs::status(filepath));
    std::cout << std::flush;
    return false;
  }
  if (access(filepath.c_str(), R_OK) != 0)
  {
    perror("Error");
    std::cout << filepath << " is an existing, regular file, " << std::endl
              << "but the program isn't permitted to read it." << std::endl;
    print_filepermissions(fs::status(filepath).permissions());
#ifdef __linux__
    std::string cmd = "ls -l " + filepath.string();
    std::cout << get_stdout_from_cmd(cmd) << std::endl;
#endif
    return false;
  }
  return true;
}

bool is_regular_write_OK(const fs::path& filepath)
{
  if (std::filesystem::exists(filepath))
  {
    if (!std::filesystem::is_regular_file(filepath))
    {
      std::cout << "Error: " << filepath << " exists and is not a regular file." << std::endl;
      print_filetype(std::cout, fs::status(filepath));
      std::cout << std::flush;
      return false;
    }
    if (access(filepath.c_str(), W_OK) != 0)
    {
      perror("Error");
      std::cout << filepath << " is an existing, regular file, " << std::endl
                << "and the program isn't permitted to write to it." << std::endl;
#ifdef __linux__
      std::string cmd = "ls -l " + filepath.string();
      std::cout << get_stdout_from_cmd(cmd) << std::endl;
#endif
      return false;
    }
  }
  return true;
}

bool has_duplicates(const std::vector<std::string>& vector, const std::string& move_kind)
{
  bool has_duplicates = false;
  for (int i = 0; i < (int) vector.size() - 1; i++)
    for (int j = i + 1; j < (int) vector.size(); j++)
      if (vector[i] == vector[j])
      {
        std::cout << "ERROR: " << move_kind << vector[i] << " is a duplicate." << std::endl;
        has_duplicates = true;
      }
  return has_duplicates;
}

bool all_in_a_exist_in_b(const std::vector<std::string>& a, const std::vector<std::string>& b, bool order_out_ref)
{
  bool success = true;
  for (const std::string& s1 : a)
  {
    //std::cout << "Move: " << s1 << std::endl;
    bool found = false;
    for (const std::string& s2 : b)
    {
      if (s1 == s2)
      {
        found = true;
        break;
      }
    }
    if (!found)
    {
      if (order_out_ref)
        std::cout << "ERROR: Output move " << s1 << " not found among reference moves." << std::endl;
      else
        std::cout << "ERROR: Reference move " << s1 << " not found among output moves." << std::endl;
      success = false;
    }
  }
  return success;
}

bool compare_move_lists(const std::vector<std::string>& out_vector, const std::vector<std::string>& ref_vector)
{
  bool success = true;
  std::string out_move;
  if (has_duplicates(out_vector, "Output move ") || has_duplicates(ref_vector, "Reference move "))
    success = false;
  if (!all_in_a_exist_in_b(out_vector, ref_vector, true))
    success = false;
  if (!all_in_a_exist_in_b(ref_vector, out_vector, false))
    success = false;
  return success;
}

std::string to_binary_board(uint64_t in)
{
  std::stringstream ss;
  uint8_t rank;
  uint64_t mask = 0x00000000000000FF;
  for (int i = 7; i >= 0; i--)
  {
    rank = in & mask;
    ss << to_binary(rank) << std::endl;
    in >>= 8;
  }
  return ss.str();
}

bool question_to_user(const std::string& question, std::string regexp_correct_answer)
{
  std::string answer;
  std::cout << question;
  getline(std::cin, answer);
  return regexp_match(answer, regexp_correct_answer);
}

std::string user_input(const std::string& message)
{
  std::string input;
  std::cout << message;
  getline(std::cin, input);
  return input;
}

} // End namespace C2_chess

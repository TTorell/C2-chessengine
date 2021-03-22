/*
 * chesstypes.cpp
 *
 *  Created on: 15 feb. 2019
 *      Author: Torsten
 *
 *  Contains some useful functions
 */
#include <iostream>
#include <regex>
#include "move.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include "shared_ostream.hpp"
#include "chesstypes.hpp"

#ifdef linux
extern "C"
{
#include <sys/wait.h>
#include <unistd.h>
}
#endif // linux

namespace C2_chess
{

using std::ostream;
using std::endl;
using std::cerr;
using std::string;
using std::regex;
using std::smatch;

#ifdef linux
ostream& print_backtrace(ostream& os)
{
    char pid_buf[30];
    sprintf(pid_buf, "--pid=%d", getpid());
    char name_buf[512];
    name_buf[readlink("/proc/self/exe", name_buf, 511)] = 0;
    int child_pid = fork();
    if (!child_pid)
    {
        dup2(2, 1); // redirect output to stderr
        os << "stack trace for " << name_buf << " pid=" << pid_buf << endl;
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

void require(bool bo, string file, string method, int line)
{
  if (!bo)
  {
    cerr << "Requirement error in " << file << ":" << method << " at line " << line << endl;
#ifdef linux
    print_backtrace(cerr) << endl;
#endif // linux

    exit(-1);
  }
}

void require_m(bool bo, string file, string method, int line, const Move &m)
{
  if (!bo)
  {
    cerr << "Move:: requirement error in " << file << ":" << method << " at line " << line << endl;
    cerr << "Move = " << m << endl << endl;
#ifdef linux
    print_backtrace(cerr) << endl;
#endif // linux

    exit(-1);
  }
}

// Run a command and return it's output as a string

#ifdef linux
#define open_pipe popen
#define close_pipe pclose
#else
#define open_pipe _popen
#define close_pipe _pclose
#endif

string GetStdoutFromCommand(string cmd)
{

  string data;
  FILE *stream;
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

bool regexp_match(const string &line, const string &regexp_string)
{
  smatch m;
  regex r(regexp_string);
  return (regex_match(line, m, r));
}

string iso_8859_1_to_utf8(const string &str)
{
  string str_out;
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

string iso_8859_1_to_utf8(const char *c_string)
{
  string str_out(c_string);
  return iso_8859_1_to_utf8(str_out);
}

} // namespace

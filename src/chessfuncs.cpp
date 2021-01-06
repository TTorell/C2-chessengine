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
#include "chesstypes.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>

extern "C"
{
#include <sys/wait.h>
#include <unistd.h>
}

namespace C2_chess
{

using std::ostream;
using std::endl;
using std::cerr;
using std::string;
using std::regex;
using std::smatch;

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

void require(bool b, string file, string method, int line)
{
  if (!b)
  {
    cerr << "Requirement error in " << file << ":" << method << " at line " << line << endl;
//      Backtrace bt;
//      bt.print();
    print_backtrace(cerr) << endl;
    exit(-1);
  }
}

void require_m(bool b, string file, string method, int line, const Move& m)
{
  if (!b)
  {
    cerr << "Move:: requirement error in " << file << ":" << method << " at line " << line << endl;
    cerr << "Move = " << m << endl << endl;
    print_backtrace(cerr) << endl;
    exit(-1);
  }
}

// Run a linux/Unix-command and return it's output as a string
string GetStdoutFromCommand(string cmd)
{

  string data;
  FILE * stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  cmd.append(" 2>&1"); // redirect stderr to same as stdin

  stream = popen(cmd.c_str(), "r");
  if (stream)
  {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL)
        data.append(buffer);
    pclose(stream);
  }
  return data;
}

bool regexp_match(const string& line, const string& regexp_string)
{
  smatch m;
  regex r(regexp_string);
  return (regex_match(line, m, r));
}
}

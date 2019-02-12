#ifndef _CHESSTYPES
#define _CHESSTYPES

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <memory>
#include <regex>

extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
}

using namespace std;

//namespace chess_vars
//{
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
const float chess_epsilon = 0.00000001;
enum piecetype
{
  King, Queen, Rook, Bishop, Knight, Pawn, Undefined
};
enum col
{
  white = 0, black = 1
};
enum player_type
{
  computer, human
};
enum output_type
{
  verbose, silent, debug, cmd_line_diagram
};
//}

//using namespace chess_vars;

#include "circular_fifo.hpp"
#include "piece.hpp"
class Square;
#include "file.hpp"
#include "rank.hpp"
#include "squarelist.hpp"
#include "square.hpp"
#include "position.hpp"
#include "move.hpp"
#include "movelist.hpp"
#include "castle_state.hpp"
#include "board.hpp"
#include "player.hpp"
#include "pgn_info.hpp"
#include "game.hpp"
#include "position_reader.hpp"
#include "backtrace.hpp"
#include "current_time.hpp"
#include "C2_unittest.hpp"

namespace chess_funcs
{
  inline ostream& print_backtrace(ostream& os)
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
      execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf,
      NULL);
      abort(); /* If gdb failed to start */
    }
    else
    {
      waitpid(child_pid,
      NULL,
              0);
    }
    return os;
  }

  inline void require(bool b, string file, string method, int line)
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

  inline void require_m(bool b, string file, string method, int line, const Move& m)
  {
    if (!b)
    {
      cerr << "Move:: requirement error in " << file << ":" << method << " at line " << line << endl;
      cerr << "Move = " << m << endl << endl;
      print_backtrace(cerr) << endl;
      exit(-1);
    }
  }

  inline string GetStdoutFromCommand(string cmd)
  {

    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");

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

  inline bool regexp_match(const string& line, const string& regexp_string)
  {
    smatch m;
    regex r(regexp_string);
    return (regex_match(line, m, r));
  }
}
using namespace chess_funcs;

#endif

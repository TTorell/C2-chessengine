/*
 * backtrace.cpp
 *
 *  Created on: 8 dec. 2018
 *      Author: torsten
 */

#include <cstdio>
#include <cstdlib>
#include "backtrace.hpp"

extern "C"
{
#include <sys/wait.h>
#include <unistd.h>
}

namespace C2_chess
{

Backtrace::Backtrace()
{
}

Backtrace::~Backtrace()
{
}

ostream& Backtrace::print(ostream& os) const
{
  char pid_buf[30];
  sprintf(pid_buf,
          "--pid=%d",
          getpid());
  char name_buf[512];
  name_buf[readlink("/proc/self/exe",
                    name_buf,
                    511)] = 0;
  int child_pid = fork();
  if (!child_pid)
  {
    dup2(2,
         1); // redirect output to stderr
    os << "stack trace for " << name_buf << " pid=" << pid_buf << endl;
//    fprintf(stdout,
//            "stack trace for %s pid=%s\n",
//            name_buf,
//            pid_buf);
    execlp("gdb",
           "gdb",
           "--batch",
           "-n",
           "-ex",
           "thread",
           "-ex",
           "bt",
           name_buf,
           pid_buf,
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

void Backtrace::print() const
{
  char pid_buf[30];
  sprintf(pid_buf,
          "--pid=%d",
          getpid());
  char name_buf[512];
  name_buf[readlink("/proc/self/exe",
                    name_buf,
                    511)] = 0;
  int child_pid = fork();
  if (!child_pid)
  {
    dup2(2,
         1); // redirect output to stderr
    fprintf(stdout,
            "stack trace for %s pid=%s\n",
            name_buf,
            pid_buf);
    execlp("gdb",
           "gdb",
           "--batch",
           "-n",
           "-ex",
           "thread",
           "-ex",
           "bt",
           name_buf,
           pid_buf,
           NULL);
    abort(); /* If gdb failed to start */
  }
  else
  {
    waitpid(child_pid,
            NULL,
            0);
  }
}


ostream& operator<<(ostream& os, const Backtrace& bt)
{
  return bt.print(os);
}
}

//
// snet-debug.c
//
// AC Keesler
//
// May 23, 2015
//
// Debugging utilities for SNET applications.
//

#include "snet-debug.h"

const char *signalNames[] = {
  "NONE",
  "SIGHUP",
  "SIGINT",
  "SIGQUIT",
  "SIGILL",
  "SIGTRAP",
  "SIGABRT",
  "SIGPOLL/SIGEMT",
  "SIGFPE",
  "SIGKILL",
  "SIGBUS",
  "SIGSEGV",
  "SIGSYS",
  "SIGPIPE",
  "SIGALRM",
  "SIGTERM",
  "SIGURG",
  "SIGSTOP",
  "SIGTSTP",
  "SIGCONT",
  "SIGCHLD",
  "SIGTTIN",
  "SIGTTOU",
  "SIGIO",
  "SIGXCPU",
  "SIGXFSZ",
  "SIGVTALRM",
  "SIGPROF",
  "SIGWINCH",
  "SIGINFO",
  "SIGUSR1",
  "SIGUSR2",
};
  


//
// server.c
//
// Andrew Keesler
//
// Created February 6. 2015
//
// Server test application.
//

#include <unit-test.h>
#include <signal.h>

#include "snet/snet.h"

static int spin = 1;

int nodeState = 0;

void handler(int signal)
{
  if (signal == SIGUSR2) {
    spin = 0;
  }
}

int main(void)
{
  signal(SIGUSR2, handler);

  while (spin) {
    usleep(500000);
  }

  return 0;
}


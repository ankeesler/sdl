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
#include "server.h"

static int spin = 1;

int nodeState = 0;

void handler(int signal)
{
  if (signal == SERVER_OFF_SIGNAL) {
    spin = 0;
  }
}

int main(void)
{
  signal(SERVER_OFF_SIGNAL, handler);

  while (spin) {
    usleep(SERVER_DUTY_CYCLE_US);
  }

  return 0;
}


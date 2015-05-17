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
#include <errno.h>

#include "mac.h"
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
  extern FILE *childLogFile;
  SdlPacket packet;

  sdlMacInit(getpid());

  signal(SERVER_OFF_SIGNAL, handler);

  fprintf(childLogFile, "server: up\n");
  while (spin) {
    usleep(SERVER_DUTY_CYCLE_US);
    // Try to receive something.
    if (sdlMacReceive(&packet) == SDL_SUCCESS) {
      spin = (packet.data[0] != SERVER_OFF_COMMAND);
    }
    fprintf(childLogFile, "errno: %d\n", errno);
  }

  exit(0);
}


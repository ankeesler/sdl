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
  SdlPacket packet;

  sdlInit(getpid());

  signal(SERVER_OFF_SIGNAL, handler);
  fprintf(stderr, "ok here we go...\n"); fflush(stderr);
  while (spin) {
    fprintf(stderr, "sleep...\n"); fflush(stderr);
    usleep(SERVER_DUTY_CYCLE_US);
    fprintf(stderr, "receive...\n"); fflush(stderr);
    // Try to receive something.
    if (sdlReceive(&packet) == SDL_SUCCESS) {
      printf("got it! command: 0x%02X\n", packet.data[0]);
      spin = (packet.data[0] != SERVER_OFF_COMMAND);
    }
  }

  return 0;
}


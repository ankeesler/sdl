//
// phy.c
//
// Andrew Keesler
//
// February 13, 2015
//
// Main code for node communicating using SDL.
//

#define __SNET_C__
#include "snet.h"
#include "phy.h"
#include "sdl-types.h"
#include "snet-internal.h"
#include "sdl-protocol.h"

#include "cap/sdl-log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// -----------------------------------------------------------------------------
// SIGNALS

// The reading fd for the pipe.
static int fd;

static void cleanup(void)
{
  // Close the read end of the pipe.
  close(fd);
  
  // Dump to the log.
  sdlLogDump();
}

static void signalHandler(int signal)
{
  if (signal == CHILD_ALERT_SIGNAL) {
    // Read the command out of the pipe.
    int command, readSize;
    if ((readSize = read(fd, &command, sizeof(command))) != sizeof(command))
      printf("(readSize:%d)", readSize); // TODO: report error.

    // Handle the command, remembering that this is
    // called in an interupt context.
    switch (command) {
    case NOOP:
      printf("(noop:%d)", getpid());
      break;
    case RECEIVE: {
      int length;
      uint8_t data[SDL_PHY_SDU_MAX];
      read(fd, &length, sizeof(int));
      read(fd, data, length);
      sdlLogRx(data, length);
      sdlPhyReceiveIsr(data, length);
    }
      break;
    default:
      printf("(command:%d)", command); // TODO: report error.
    }
  } else if (signal == CHILD_QUIT_SIGNAL) {
    cleanup();
    exit(0);
  }
}

// -----------------------------------------------------------------------------
// MAIN

int main(int argc, char *argv[])
{
  int ret;

  // The first argument past the program name is pipe fds from the
  // parent.
#ifndef SNET_TEST
  if (argc < 2) {
    return 1;
  } else {
    uint8_t i;
    for (fd = i = 0; i < (sizeof(int) * 2); i ++)
      fd |= (0x0F & argv[1][i]) << (i<<2);
  }
#endif

  // Say that we want to handle the CHILD_ALERT_SIGNAL signal.
  // This will be our parent telling us that there is data for
  // us in the pipe.
  signal(CHILD_ALERT_SIGNAL, signalHandler);
  
  // Also say that we want to handle the CHILD_QUIT_SIGNAL,
  // for when our parent wants us to quit immediately.
  signal(CHILD_QUIT_SIGNAL, signalHandler);

  // Call the child node's main function.
  ret = SNET_MAIN(argc, argv);

  // When done, cleanup.
  cleanup();

  exit(ret);
}

// -----------------------------------------------------------------------------
// PHY INTERFACE

static uint8_t realTxBuffer[SDL_PHY_SDU_MAX + SDL_PHY_PDU_LENGTH];

SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length)
{
  uint8_t realLength = length + SDL_PHY_PDU_LENGTH;
  
  // Copy to real tx buffer.
  realTxBuffer[0] = realLength;
  memcpy(realTxBuffer + 1, data, length);
  
  // Log.
  sdlLogTx(realTxBuffer, realLength);
  
  // TODO:
  
  return SDL_SUCCESS;
}
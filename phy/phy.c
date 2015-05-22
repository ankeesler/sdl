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

FILE *childLogFile = NULL;

// The reading fd for the pipe.
static int fd;

static void cleanup(void)
{
  // Close our log file.
  fclose(childLogFile);

  // Close the read end of the pipe.
  close(fd);
  
  // Dump to the log.
  sdlLogDump();
}

static void signalHandler(int signal)
{
  uint8_t rxBuffer[SDL_PHY_SDU_MAX + SDL_PHY_PDU_LEN];
  SnetNodeCommand command = NOOP;

  if (signal == CHILD_ALERT_SIGNAL) {
    // Read the command out of the pipe.
    // TODO: report bad read.
    //read(fd, &command, sizeof(command));

    // Handle the command, remembering that this is
    // called in an interupt context.
    switch (command) {
    case NOOP:
      break;
    case RECEIVE:
      // First byte is the PHY PDU, i.e., the length of the whole packet.
      read(fd, rxBuffer, sizeof(uint8_t));
      read(fd, rxBuffer + 1, rxBuffer[0] - 1);
      sdlLogRx(rxBuffer, rxBuffer[0]);
      sdlPhyReceiveIsr(rxBuffer + 1, rxBuffer[0] - 1);
      break;
    default:
      ; // TODO: report bad command.
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
#ifndef SDL_LOG_TEST
  if (argc < 2) {
    return 1;
  } else {
    // This is our file descriptor for communicating with our parent.
    fd = atoi(argv[1]);
  }
#endif

  // Log file.
  childLogFile = fopen(".child", "w");
  fprintf(childLogFile, "HELLO\n");
  fprintf(childLogFile, "I am %s and my fd is %d\n", argv[0], fd);

  // Say that we want to handle the CHILD_ALERT_SIGNAL signal.
  // This will be our parent telling us that there is data for
  // us in the pipe.
  signal(CHILD_ALERT_SIGNAL, signalHandler);
  fprintf(childLogFile, "registering CHILD_ALERT_SIGNAL\n");
  
  // Also say that we want to handle the CHILD_QUIT_SIGNAL,
  // for when our parent wants us to quit immediately.
  signal(CHILD_QUIT_SIGNAL, signalHandler);
  fprintf(childLogFile, "registering CHILD_QUIT_SIGNAL\n");

  // Call the child node's main function.
  ret = SNET_MAIN(argc, argv);

  // When done, cleanup.
  cleanup();
 
  exit(ret);
}

// -----------------------------------------------------------------------------
// PHY INTERFACE

static uint8_t realTxBuffer[SDL_PHY_SDU_MAX + SDL_PHY_PDU_LEN];

SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length)
{
  uint8_t realLength = length + SDL_PHY_PDU_LEN;
  
  // Copy to real tx buffer.
  realTxBuffer[0] = realLength;
  memcpy(realTxBuffer + 1, data, length);
  
  // Log.
  sdlLogTx(realTxBuffer, realLength);
  
  // TODO:
  
  return SDL_SUCCESS;
}

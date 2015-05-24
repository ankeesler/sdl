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
#include "snet-debug.h"

#include "cap/sdl-log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// SIGNALS

FILE *childLogFile = NULL;
char childLogFilename[256];

// The reading fd for the pipe.
static int fd;

static void cleanup(void)
{
  fprintf(childLogFile, "GOODBYE\n");

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
  SnetNodeCommand command = NIL;

  fprintf(childLogFile, "signal: %s\n", signalNames[signal]);

  if (signal == CHILD_ALERT_SIGNAL) {
    // Read the command out of the pipe.
    // TODO: report bad read.
    read(fd, &command, sizeof(command));

    // Handle the command, remembering that this is
    // called in an interupt context.
    switch (command) {
    case NOOP:
      break;
    case RECEIVE:
    case TRANSMIT:
      // First byte is the PHY PDU, i.e., the length of the whole packet.
      read(fd, rxBuffer, sizeof(uint8_t));
      read(fd, rxBuffer + 1, rxBuffer[0] - SDL_PHY_PDU_LEN);
      if (command == RECEIVE) {
        sdlLogRx(rxBuffer, rxBuffer[0]);
        sdlPhyReceiveIsr(rxBuffer + 1, rxBuffer[0] - SDL_PHY_PDU_LEN);
      } else { // command == TRANSMIT
        // TODO: pass me to sdlPhyTransmit!
        // sdlPhyTransmit(rxBuffer + 1, rxBuffer[0] - SDL_PHY_PDU_LEN);
      }
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
    sscanf(argv[PARENT_TO_CHILD_FD_INDEX], "%d", &fd);
  }
#endif

  // Log file.
  sprintf(childLogFilename, ".child-%s", argv[CHILD_NAME_INDEX]);
  childLogFile = fopen(childLogFilename, "w");
  fprintf(childLogFile, "HELLO\n");
  fprintf(childLogFile, "name: %s, fd: %d\n", argv[CHILD_NAME_INDEX], fd);

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

static uint8_t realTxBuffer[SDL_PHY_SDU_MAX + SDL_PHY_PDU_LEN];

SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length)
{
  SdlStatus status = SDL_SUCCESS;
  uint8_t realLength = length + SDL_PHY_PDU_LEN;
  
  // Copy to real tx buffer.
  realTxBuffer[0] = realLength;
  memcpy(realTxBuffer + 1, data, length);
  
  // Log.
  sdlLogTx(realTxBuffer, realLength);
  
  // Write the packet to the file descriptor.
#ifndef SDL_LOG_TEST /* TODO: take me away when log-test moves to SNET. */
  status = (write(fd, realTxBuffer, realTxBuffer[0]) == realTxBuffer[0]
            ? SDL_SUCCESS
            : SDL_TRANSMIT_FAILURE);
#endif  

  // Tell the parent they have got something coming for them.
  return (status == SDL_SUCCESS
          ? (snetParentAlert()
             ? SDL_TRANSMIT_FAILURE
             : SDL_SUCCESS)
          : status);
}

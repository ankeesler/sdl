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
// DEFINITIONS

#define sdlPacketLength(buffer)     (buffer[0])

#define sdlPacketData(buffer)       (buffer + SDL_PHY_PDU_LEN)
#define sdlPacketDataLength(buffer) (buffer[0] - SDL_PHY_PDU_LEN)

// -----------------------------------------------------------------------------
// SIGNALS

FILE *childLogFile = NULL;
char childLogFilename[256];
#define childLog(format, ...)                         \
  do {                                                \
    fprintf(childLogFile, format "\n", __VA_ARGS__);  \
    fflush(childLogFile);                             \
  } while (0);

static int parentToChildFd, childToParentFd;

static void cleanup(void)
{
  childLog("%s", "GOODBYE");

  // Close our log file.
  fclose(childLogFile);

  // Close the pipes.
  close(parentToChildFd);
  close(childToParentFd);
  
  // Dump to the log.
  sdlLogDump();
}

static void signalHandler(int signal)
{
  uint8_t rxBuffer[SDL_PHY_SDU_MAX + SDL_PHY_PDU_LEN];
  SnetNodeCommand command = NIL;

  childLog("signal: %s", signalNames[signal]);

  if (signal == CHILD_ALERT_SIGNAL) {
    // Read the command out of the pipe.
    if (read(parentToChildFd, &command, sizeof(command)) != sizeof(command)) {
      cleanup();
      exit(CHILD_EXIT_BAD_READ);
    }
    childLog("  command: 0x%02X", command);

    // Handle the command, remembering that this is
    // called in an interupt context.
    switch (command) {
    case NOOP:
      break;
    case RECEIVE:
    case TRANSMIT:
      // First byte is the PHY PDU, i.e., the length of the whole packet.
      read(parentToChildFd,
           rxBuffer,
           SDL_PHY_PDU_LEN);
      read(parentToChildFd,
           rxBuffer + SDL_PHY_PDU_LEN,
           sdlPacketDataLength(rxBuffer));
      if (command == RECEIVE) {
        sdlLogRx(rxBuffer, sdlPacketLength(rxBuffer));
        sdlPhyReceiveIsr(sdlPacketData(rxBuffer), sdlPacketDataLength(rxBuffer));
      } else { // command == TRANSMIT
        sdlPhyTransmit(sdlPacketData(rxBuffer), sdlPacketDataLength(rxBuffer));
      }
      break;
    default:
      cleanup();
      exit(CHILD_EXIT_BAD_COMMAND);
    }
  } else if (signal == CHILD_QUIT_SIGNAL) {
    cleanup();
    exit(CHILD_EXIT_SUCCESS);
  }
}

// -----------------------------------------------------------------------------
// MAIN

int main(int argc, char *argv[])
{
  int ret;

#if !defined(SDL_LOG_TEST)
  if (argc < 2) {
    return 1;
  } else {
    sscanf(argv[PARENT_TO_CHILD_FD_INDEX], "%d", &parentToChildFd);
    sscanf(argv[CHILD_TO_PARENT_FD_INDEX], "%d", &childToParentFd);
  }
#endif

  // Log file.
  sprintf(childLogFilename, ".child-%s", argv[CHILD_NAME_INDEX]);
  childLogFile = fopen(childLogFilename, "w");
  childLog("%s", "HELLO");
  childLog("name: %s, p -> c: %d, c -> p %d\n",
           argv[CHILD_NAME_INDEX],
           parentToChildFd,
           childToParentFd);

  // Say that we want to handle the CHILD_ALERT_SIGNAL signal.
  // This will be our parent telling us that there is data for
  // us in the pipe.
  signal(CHILD_ALERT_SIGNAL, signalHandler);
  
  // Also say that we want to handle the CHILD_QUIT_SIGNAL,
  // for when our parent wants us to quit immediately.
  signal(CHILD_QUIT_SIGNAL, signalHandler);

  // Initialize logging. If logging is not used, this will be macro'd out.
  sdlLogInit();

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
  status = ((write(childToParentFd,
                   realTxBuffer,
                   realTxBuffer[0])
             == realTxBuffer[0])
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

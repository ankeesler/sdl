//
// phy.c
//
// Andrew Keesler
//
// February 13, 2015
//
// Main code for node communicating using SDL.
//

#include <stdint.h> // uint8_t, UINT8_MAX
#include <signal.h> // sig_atmoic_t
#include <assert.h> // assert()

#include "snet/src/common/snet-command.h" // SNET_CHILD_COMMAND_*
#include "snet/src/common/child-data.h"   // snetChildDataSend
#include "snet/src/child/child-main.h"    // snetChildToParentFd

#include "sdl-protocol.h"
#include "sdl-types.h"

#include "phy-internal.h"
#include "plat/nvic.h"
#include "cap/sdl-log.h"

// -----------------------------------------------------------------------------
// Globals

volatile sig_atomic_t _transmitSnetErrno;

// -----------------------------------------------------------------------------
// Public API

SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length)
{
  SdlStatus status;

  _transmitSnetErrno = snetChildDataSend(snetChildToParentFd,
                                         getpid(),
                                         SNET_CHILD_COMMAND_NETIF_TRANSMIT,
                                         length,
                                         data);
  status = (snetErrnoUnix(_transmitSnetErrno) != 0 ? SDL_FATAL : SDL_SUCCESS);

  snetChildLogPrintf(snetChildLog,
                     "Transmit with status 0x%08X.\n",
                     _transmitSnetErrno);
  snetChildLogPrintBytes(snetChildLog, data, length);

  if (status == SDL_SUCCESS) {
    sdlLogTx(data, length);
  }

  return status;
}

// -----------------------------------------------------------------------------
// Stack Internal API

SdlStatus phyInit(void)
{
  assert(sdlLogInit() == 0);

  return SDL_SUCCESS;
}

// -----------------------------------------------------------------------------
// Receive API

void nvicNetifReceiveIsr(uint8_t *data, uint8_t dataLength)
{
  sdlLogRx(data, dataLength);
  phyReceiveIsr(data, dataLength);
}

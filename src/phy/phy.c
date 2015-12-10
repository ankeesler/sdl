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
#include <string.h> // memcpy()

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

static uint8_t _transmitBuffer[SDL_PHY_SDU_MAX];

// -----------------------------------------------------------------------------
// Public API

SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length)
{
  SdlStatus status;
  uint8_t transmitLength = length + SDL_PHY_PDU_LEN;

  memcpy(&_transmitBuffer[SDL_PHY_PDU_LEN], data, length);
  _transmitBuffer[0] = transmitLength;

  _transmitSnetErrno = snetChildDataSend(snetChildToParentFd,
                                         getppid(),
                                         SNET_CHILD_COMMAND_NETIF_TRANSMIT,
                                         transmitLength,
                                         _transmitBuffer);
  status = (snetErrnoUnix(_transmitSnetErrno) != 0
            ? SDL_TRANSMIT_FAILURE
            : SDL_SUCCESS);

  snetChildLogPrintf(snetChildLog,
                     "Transmit with status 0x%08X.\n",
                     _transmitSnetErrno);
  snetChildLogPrintBytes(snetChildLog, _transmitBuffer, transmitLength);

  if (status == SDL_SUCCESS) {
    sdlLogTx(_transmitBuffer, transmitLength);
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
  phyReceiveIsr(&data[SDL_PHY_PDU_LEN], dataLength - SDL_PHY_PDU_LEN);
}

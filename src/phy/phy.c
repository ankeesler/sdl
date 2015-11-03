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

#include "snet/src/child/child-main.h"    // snetChildSignalHandler()
#include "snet/src/common/child-data.h"   // SNET_CHILD_SIGNAL_ALERT
#include "snet/src/common/snet-errno.h"   // SnetErrno_t
#include "snet/src/common/snet-command.h" // SNET_CHILD_COMMAND_*

#include "sdl-protocol.h"
#include "sdl-types.h"

#include "phy-internal.h"
#include "cap/sdl-log.h"

// -----------------------------------------------------------------------------
// Globals

// These are the signal-safe global receive values.
volatile sig_atomic_t _childCommand, _childPayloadLength;
volatile sig_atomic_t _childPayload[(UINT8_MAX + 1) / sizeof(sig_atomic_t)];

// These are the signal-safe global transmit and receive errors.
volatile sig_atomic_t _transmitSnetErrno, _receiveSnetErrno;

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

void snetChildSignalHandler(int signal)
{
  if (signal == SNET_CHILD_SIGNAL_ALERT) {
    _receiveSnetErrno = snetChildDataReceive(snetParentToChildFd,
                                             (uint8_t *)&_childCommand,
                                             (uint8_t *)&_childPayloadLength,
                                             (uint8_t *)&_childPayload);
    // If we get an error, all we can do is write it to log.
    if (_receiveSnetErrno != 0) {
      snetChildLogPrintf(snetChildLog,
                         "Error: snetChildSignalHandler 0x%04X (0x%02X, 0x%02X).",
                         _receiveSnetErrno, _childCommand, _childPayloadLength);
      return;
    }

    // Process the command.
    if (_childCommand == SNET_CHILD_COMMAND_NETIF_RECEIVE) {
      uint8_t *data = (uint8_t *)&_childPayload;
      uint8_t dataLength = _childPayloadLength & 0xFF;
      sdlLogRx(data, dataLength);
      phyReceiveIsr(data, dataLength);
    }
  }
}

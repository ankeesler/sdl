//
// nvic.c
//
// Andrew Keesler
//
// November 16, 2015
// Flight back from Charlotte to Boston.
//
// Pseudo-ISR controller. Link from simulated network to stack.
//

#include <stdlib.h> // exit()

#include "src/snet/src/common/snet-command.h" // SNET_CHILD_COMMAND_*
#include "src/snet/src/common/child-data.h"   // SNET_CHILD_SIGNAL_ALERT, snetChildData*
#include "src/snet/src/child/child-log.h"     // snetChildLogPrint*
#include "src/snet/src/child/child-main.h"    // snetChildSignalHandler

#include "cap/sdl-log.h"
#include "led.h"
#include "assert.h"

#include "nvic.h"

// -----------------------------------------------------------------------------
// Globals

// These are the signal-safe global receive values.
volatile sig_atomic_t _childCommand, _childPayloadLength;
volatile sig_atomic_t _childPayload[(UINT8_MAX + 1) / sizeof(sig_atomic_t)];

// These are the signal-safe global transmit and receive errors.
volatile sig_atomic_t _receiveSnetErrno;

// -----------------------------------------------------------------------------
// SNET pseudo-ISR

void snetChildSignalHandler(int signal)
{
  if (signal == SNET_CHILD_SIGNAL_ALERT) {
    uint8_t *data;
    uint8_t dataLength;

    // Read the data out of the file descriptor.
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
    } else {
      snetChildLogPrintf(snetChildLog,
                         "Receive command 0x%02X.\n",
                         _childCommand);
      snetChildLogPrintBytes(snetChildLog,
                             (uint8_t *)_childPayload,
                             (uint8_t)(_childPayloadLength & 0xFF));
    }

    // Dispatch the command.
    data = (uint8_t *)&_childPayload;
    dataLength = _childPayloadLength & 0xFF;
    switch (_childCommand) {
    case SNET_CHILD_COMMAND_NETIF_RECEIVE:
      nvicNetifReceiveIsr(data, dataLength);
      break;
    case SNET_CHILD_COMMAND_LED_SET:
    case SNET_CHILD_COMMAND_LED_CLEAR:
      nvicLedTouchedIsr(data[0], (_childCommand == SNET_CHILD_COMMAND_LED_SET));
      break;
    case SNET_CHILD_COMMAND_LED_READ:
      data[0] = sdlPlatLedRead(data[0]);
      sdlPlatAssert(snetChildDataSend(snetChildToParentFd,
                                      getppid(),
                                      _childCommand,
                                      1, // length
                                      data)
                    == 0);
      break;
    case SNET_CHILD_COMMAND_UART_RECEIVE:
      nvicUartIsr(data, dataLength);
      break;

    default:
      ; // uh?
    }
  } else if (signal == SNET_CHILD_SIGNAL_SIGTERM) {
    sdlLogDump();
    exit(SNET_CHILD_EXIT_STATUS_SUCCESS);
  }
}



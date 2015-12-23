//
// serial.c
//
// Andrew Keesler
//
// Tuesday, December 8, 2015
//
// Serial implementation.
//

#include "serial.h"

#include <stdarg.h> // va_list, va_start(), va_end(), va_arg()
#include <stdint.h> // uint32_t
#include <unistd.h> // getppid()
#include <string.h> // memcpy()
#include <signal.h> // sig_atomic_t

#include "snet/src/common/child-data.h"   // snetChildDataSend()
#include "snet/src/common/snet-command.h" // SNET_CHILD_COMMAND_UART_RECEIVE
#include "snet/src/common/snet-errno.h"   // SnetErrno_t
#include "snet/src/child/child-main.h"    // snetChildToParentFd, snetChildLog
#include "snet/src/child/child-log.h"     // snetChildLogPrintf

#include "inc/sdl-types.h" // SdlCounter
#include "nvic.h"

static uint8_t txBuffer[255];
#define TX_BUFFER_LENGTH sizeof(txBuffer)

// The first byte of each rx buffer is the length.
// The second byte of each rx buffer is the current read index.
// The third byte is the status of the buffer.
// The data starts on the fourth byte.
#define RX_BUFFER_LENGTH_OFFSET      (0)
#define RX_BUFFER_READ_OFFSET        (1)
#define RX_BUFFER_DATA_OFFSET        (2)

#define RX_BUFFER_SIZE_LOG_4 \
  ((SDL_PLAT_SERIAL_RX_BUFFER_SIZE >> 2) + 2)
static volatile sig_atomic_t rxBuffers[2][RX_BUFFER_SIZE_LOG_4];
static volatile sig_atomic_t rxStatusMask = 0;
#define RX_BUFFER_STATUS_LOAD_A (1 << 0)
#define RX_BUFFER_STATUS_READ_A (1 << 1)
#define RX_BUFFER_STATUS_LOAD_B (1 << 2)
#define RX_BUFFER_STATUS_READ_B (1 << 3)
#define RX_BUFFER_STATUS_PRIO_A (1 << 4)

// The code should use these symbols to access the rx data.
#define rxBufferA ((uint8_t *)rxBuffers[0])
#define rxBufferB ((uint8_t *)rxBuffers[1])
#define RX_STATUS(bit) (rxStatusMask & RX_BUFFER_STATUS_ ## bit)
  
volatile sig_atomic_t platSerialRxOverflow = 0, platSerialRxDrop = 0;

SdlStatus sdlPlatSerialPrintf(char *format, ...)
{
  va_list args;
  uint32_t printLength;
  SdlStatus status = SDL_SIZE_ERR;
  
  va_start(args, format);
  printLength = vsnprintf((char *)txBuffer,
                          TX_BUFFER_LENGTH,
                          format,
                          args);
  va_end(args);

  if (printLength < TX_BUFFER_LENGTH) {
    SnetErrno_t snetErrno = snetChildDataSend(snetChildToParentFd,
                                              getppid(),
                                              SNET_CHILD_COMMAND_UART_RECEIVE,
                                              printLength,
                                              txBuffer);
    status = (snetErrno == 0 ? SDL_SUCCESS : SDL_SNET_COM_FAILURE);
  }

  return status;
}

SdlStatus sdlPlatSerialRead(uint8_t *byte)
{
  uint8_t *rxBufferLoc;
  uint8_t rxBufferReadIndex;
  
  // First, try to find the buffer that we are reading from.
  // Else, if we are not currently reading from a buffer, find a buffer that
  // is loaded.
  if (RX_STATUS(READ_A)) {
    rxBufferLoc = rxBufferA;
  } else if (RX_STATUS(READ_B)) {
    rxBufferLoc = rxBufferB;
  } else if (RX_STATUS(LOAD_A) && (!RX_STATUS(LOAD_B) || RX_STATUS(PRIO_A))) {
    rxBufferLoc = rxBufferA;
  } else if (RX_STATUS(LOAD_B)) {
    rxBufferLoc = rxBufferB;
  } else {
    // If nothing is loaded, then we don't have anything for the user to read.
    return SDL_SERIAL_EMPTY;
  }
  
  // If the next index we should be reading is past the end of the rx data,
  // then we return empty.
  rxBufferReadIndex = rxBufferLoc[RX_BUFFER_READ_OFFSET];
  if (rxBufferReadIndex
      >= rxBufferLoc[RX_BUFFER_LENGTH_OFFSET] + RX_BUFFER_DATA_OFFSET) {
    rxStatusMask &= ~(rxBufferLoc == rxBufferA
                      ? (RX_BUFFER_STATUS_READ_A
                         | RX_BUFFER_STATUS_LOAD_A)
                      : (RX_BUFFER_STATUS_READ_B
                         | RX_BUFFER_STATUS_LOAD_B));
    return SDL_SERIAL_EMPTY;
  }
  
  // Set the byte, increment the read index, and make sure we note that 
  // we are reading this buffer.
  *byte = rxBufferLoc[rxBufferReadIndex];
  rxBufferLoc[RX_BUFFER_READ_OFFSET] ++;
  rxStatusMask |= (rxBufferLoc == rxBufferA
                   ? RX_BUFFER_STATUS_READ_A
                   : RX_BUFFER_STATUS_READ_B);

  return SDL_SUCCESS;
}

void nvicUartIsr(uint8_t *data, uint8_t dataLength)
{
  uint8_t *rxBufferLoc;
  
  // If we receive too much data, note that.
  if (dataLength > SDL_PLAT_SERIAL_RX_BUFFER_SIZE) {
    dataLength = SDL_PLAT_SERIAL_RX_BUFFER_SIZE;
    platSerialRxOverflow ++;
  }
  
  // Find an available buffer. Try to find a buffer that is not loaded first,
  // and if you can't find one of those then try to find a buffer that you
  // are not reading from.
  if (!(RX_STATUS(LOAD_A))) {
    rxBufferLoc = rxBufferA;
  } else if (!RX_STATUS(LOAD_B)) {
    rxBufferLoc = rxBufferB;
  } else {
    if (!(RX_STATUS(READ_A))) {
      rxBufferLoc = rxBufferA;
    } else {
      rxBufferLoc = rxBufferB;
    }
    // Keep track of any dropped rx buffers.
    platSerialRxDrop ++;
  }

  // Update the status mask.
  if (rxBufferLoc == rxBufferA) {
    rxStatusMask |= RX_BUFFER_STATUS_LOAD_A;
    if (RX_STATUS(LOAD_B)) {
      rxStatusMask &= ~RX_BUFFER_STATUS_PRIO_A;
    }
  } else { // rxBufferLoc == rxBufferB
    rxStatusMask |= RX_BUFFER_STATUS_LOAD_B;
    if (RX_STATUS(LOAD_A)) {
      rxStatusMask |= RX_BUFFER_STATUS_PRIO_A;
    }
  }
  
  // Initialize the buffer with the length and load the data in.
  rxBufferLoc[RX_BUFFER_LENGTH_OFFSET] = dataLength;
  rxBufferLoc[RX_BUFFER_READ_OFFSET] = RX_BUFFER_DATA_OFFSET;
  memcpy(&rxBufferLoc[RX_BUFFER_DATA_OFFSET], data, dataLength);
}

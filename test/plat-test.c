//
// plat-test.c
//
// Andrew Keesler
//
// Sunday, December 13, 2015
//
// SDL serial test.
//

#include <unit-test.h>

#include <stdint.h> // uint32_t

#include "snet/src/common/snet-errno.h" // SnetErrno_t, SNET_ERRNO_SUCCESS
#include "snet/src/child/child-log.h"   // SnetChildLog_t

#include "inc/sdl.h"
#include "src/plat/nvic.h"
#include "src/plat/serial-internal.h"

// -----------------------------------------------------------------------------
// Stubs

SnetErrno_t snetChildDataSend(int fd,
                              pid_t childPid,
                              uint8_t command,
                              uint8_t payloadLength,
                              uint8_t *payload)
{
  return SNET_ERRNO_SUCCESS;
}

SnetErrno_t snetChildDataReceive(int fd,
                                 uint8_t *command,
                                 uint8_t *payloadLength,
                                 uint8_t *payload)
{
  return SNET_ERRNO_SUCCESS;
}

SnetChildLog_t *snetChildLog = NULL;
int snetChildToParentFd, snetParentToChildFd;

void snetChildLogPrintf(SnetChildLog_t *log,
                        const char *format,
                        ...)
{

}

void snetChildLogPrintBytes(SnetChildLog_t *log,
                            uint8_t *bytes,
                            uint8_t count)
{

}

void nvicNetifReceiveIsr(uint8_t *data, uint8_t dataLength)
{
}

int sdlLogDump(void)
{
  return 0;
}

// -----------------------------------------------------------------------------
// Tests

static uint8_t data5[] = {0, 1, 2, 3, 4};
static uint8_t data7[] = {1, 3, 5, 7, 9, 11, 13};
static uint8_t data10[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static uint8_t dataOverflow[SDL_PLAT_SERIAL_RX_BUFFER_SIZE + 1];

#define expectSerialRead(byte, value)                    \
  expectEquals(sdlPlatSerialRead(&(byte)), SDL_SUCCESS); \
  expectEquals((byte), (value));

static void reallySerialTest(uint8_t errorCount)
{
  uint8_t byte;
  uint32_t i;
  
  // Having nothing in the rx buffer should mean we should get
  // an SDL_SERIAL_EMPTY;
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  
  // The ISR is called, and we should be able to receive the byte.
  nvicUartIsr(data5, sizeof(data5));
  for (i = 0; i < sizeof(data5); i ++) {
    expectSerialRead(byte, data5[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  
  // If too many bytes were received, we should kick a counter.
  expectEquals(platSerialRxOverflow, errorCount);
  nvicUartIsr(dataOverflow, sizeof(dataOverflow));
  expectEquals(platSerialRxOverflow, errorCount + 1);
  
  // Read all of the data overflow rx bytes.
  while (sdlPlatSerialRead(&byte) == SDL_SUCCESS) ;
  
  // Here comes some more data...
  nvicUartIsr(data10, sizeof(data10));
  for (i = 0; i < (sizeof(data10) / 2); i ++) {
    expectSerialRead(byte, data10[i]);
  }
  
  // ...for which the reading is messed up with more data. We can still
  // successfully read the rest of the first chunk of data, and then
  // we read this new chunk of data.
  nvicUartIsr(data5, sizeof(data5));
  for (i = sizeof(data10) / 2; i < sizeof(data10); i ++) {
    expectSerialRead(byte, data10[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  for (i = 0; i < sizeof(data5); i ++) {
    expectSerialRead(byte, data5[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  
  // When we receive two isr's before we are able to read anything, we should
  // handle these as above.
  nvicUartIsr(data5, sizeof(data5));
  nvicUartIsr(data10, sizeof(data10));
  for (i = 0; i < sizeof(data5); i ++) {
    expectSerialRead(byte, data5[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  for (i = 0; i < sizeof(data10); i ++) {
    expectSerialRead(byte, data10[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  
  // If we receive more data when we already have 2 rx buffers, we drop one.
  // Make sure that we are reading the oldest rx buffer first.
  // We should also mark this drop in a counter.
  expectEquals(platSerialRxDrop, errorCount);
  nvicUartIsr(data10, sizeof(data10));
  nvicUartIsr(data7, sizeof(data7));
  nvicUartIsr(data5, sizeof(data5));
  for (i = 0; i < sizeof(data7); i ++) {
    expectSerialRead(byte, data7[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  for (i = 0; i < sizeof(data5); i ++) {
    expectSerialRead(byte, data5[i]);
  }
  expectEquals(sdlPlatSerialRead(&byte), SDL_SERIAL_EMPTY);
  expectEquals(platSerialRxDrop, errorCount + 1);
}

static int serialTest(void)
{
  // Run the test twice to try to catch messed up positive behavior after
  // negative behavior.
  reallySerialTest(0);
  reallySerialTest(1);
  
  return 0;
}

int main(void)
{
  announce();
  
  run(serialTest);

  return 0;
}

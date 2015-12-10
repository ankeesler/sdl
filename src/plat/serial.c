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

#include "common/child-data.h"   // snetChildDataSend()
#include "common/snet-command.h" // SNET_CHILD_COMMAND_UART_RECEIVE
#include "common/snet-errno.h"   // SnetErrno_t
#include "child/child-main.h"    // snetChildToParentFd, snetChildLog
#include "child/child-log.h"     // snetChildLogPrintf

static uint8_t txBuffer[255];

#define TX_BUFFER_LENGTH sizeof(txBuffer)

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

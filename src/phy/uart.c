//
// uart.c
//
// Andrew Keesler
//
// July 13, 2015
//
// UART driver for simulated node.
//

#include "sdl-types.h"

#include "snet-internal.h"
#include "phy-internal.h"

#include <unistd.h> // write

SdlStatus sdlUartTransmit(uint8_t *data, uint8_t length)
{
  SdlStatus status;
  uint8_t command = CHILD_TO_PARENT_COMMAND_UART;

  // Write the command to the file descriptor.
  status = ((write(childToParentFd,
                   &command,
                   sizeof(command))
             == sizeof(command))
            ? SDL_SUCCESS
            : SDL_TRANSMIT_FAILURE);

  // Write the length to the file descriptor.
  if (status == SDL_SUCCESS) {
    status = ((write(childToParentFd,
                     &length,
                     sizeof(length))
               == sizeof(length))
              ? SDL_SUCCESS
              : SDL_TRANSMIT_FAILURE);
  }

  // Write the data to the file descriptor.
  if (status == SDL_SUCCESS) {
    status = ((write(childToParentFd,
                     data,
                     length)
               == length)
              ? SDL_SUCCESS
              : SDL_TRANSMIT_FAILURE);
  }

  return (status == SDL_SUCCESS
          ? (snetParentAlert()
             ? SDL_TRANSMIT_FAILURE
             : SDL_SUCCESS)
          : status);
}

//
// serial.h
//
// Andrew Keesler
//
// Tuesday, December 8, 2015
//
// Serial interface.
//

#include "sdl-types.h" // SdlStatus

#include <stdint.h> // uint8_t

#ifndef SDL_PLAT_SERIAL_RX_BUFFER_SIZE
  #define SDL_PLAT_SERIAL_RX_BUFFER_SIZE (64)
#endif

#if SDL_PLAT_SERIAL_RX_BUFFER_SIZE > 253
 #error SDL_PLAT_SERIAL_RX_BUFFER_SIZE may not be greater than 254.
#endif

// Print something over the UART.
SdlStatus sdlPlatSerialPrintf(char *format, ...);

// Read a byte out of the UART.
SdlStatus sdlPlatSerialRead(uint8_t *byte);

//
// serial.h
//
// Andrew Keesler
//
// Tuesday, December 8, 2015
//
// Serial interface.
//

#include "sdl-types.h"

// Print something over the UART.
SdlStatus sdlPlatSerialPrintf(char *format, ...);

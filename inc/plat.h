//
// plat.h
//
// Andrew Keesler
//
// July 18, 2015
//
// SDL platform utilities.
//

#include "sdl-types.h"

// A button ISR. Good for testing.
void sdlPhyButtonIsr(uint8_t button);

// Transmit some bytes across a UART.
SdlStatus sdlUartTransmit(uint8_t *data, uint8_t length);



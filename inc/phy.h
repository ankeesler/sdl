//
// phy.h
//
// Andrew Keesler
//
// May 16, 2015
//
// SDL PHY interface upwards and defines.
//

#ifndef __PHY_H__
#define __PHY_H__

#include "sdl-types.h"

#include <stdint.h>

// Transmit length number of bytes in data.
SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length);

// A button ISR. Good for testing.
void sdlPhyButtonIsr(uint8_t button);

#endif /* __PHY_H__ */

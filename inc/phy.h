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

// Transmit length number of bytes in data.
SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length);

#endif /* __PHY_H__ */

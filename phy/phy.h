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

// The physical layer has received length number of bytes.
// The data points to the first byte past the PHY PDU.
// The length is the length of the data vector.
void sdlPhyReceiveIsr(uint8_t *data, uint8_t length);

#endif /* __PHY_H__ */
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

// The physical layer has received length number of bytes.
void sdlPhyReceiveIsr(uint8_t *data, uint8_t length);

#endif /* __PHY_H__ */
//
// mac.h
//
// Andrew Keesler
//
// November 16, 2014
//
// SDL MAC layer interface.
//

#ifndef __SDL_H__
#define __SDL_H__

#include "sdl-types.h"

//
// Management.
//

// Initialize the simulated data link layer.
// Address is meant to be a completely unique address for the node.
// Returns a sdl status value with the result of the initialization routine.
SdlStatus sdlMacInit(SdlAddress address);

// Returns the address of this node.
// This address is meant to be completely unique.
// This will return SDL_UNINITIALIZED if the SDL layer has not been initialized.
SdlStatus sdlMacAddress(SdlAddress *address);

//
// Communication.
//

// Does not block.
// Argument dataLength is in bytes.
// Will return SDL_UNINITIALIZED if the SDL has not been initialized yet.
SdlStatus sdlMacTransmit(SdlPacketType type,
                         SdlAddress destination,
                         uint8_t *data,
                         uint8_t dataLength);

// Does not block.
// Creates a deep copy of a packet to receive.
// If there is nothing to receive, this will return SDL_EMPTY.
// If the SDL layer is not initialized yet, this will return SDL_UNINITIALIZED.
SdlStatus sdlMacReceive(SdlPacket *packet);

// Do something kinda like a CCA.
uint8_t sdlMacActivity(void);

//
// Utilities
//

// Takes an SdlPacket struct and converts it to a flat buffer.
void sdlPacketToFlatBuffer(SdlPacket *packet, uint8_t *buffer);

// Get the current value of a SdlCounter.
// Returns true if the counter has a value and false if there was
// no value for the counter.
bool sdlCounterValue(SdlCounter counter, uint32_t *value);

// Clear the current counter value of an SdlCounter.
// Returns true if the counter has a value and false if there was
// no value for the counter.
bool sdlCounterClear(SdlCounter counter);

#endif /* __SDL_H__ */

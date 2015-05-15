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

#include "sdl-protocol.h"
#include "sdl-types.h"

//
// Management.
//

// Initialize the simulated data link layer.
// Address is meant to be a completely unique address for the node.
// Returns a sdl status value with the result of the initialization routine.
SdlStatus sdlInit(SdlAddress address);

// Returns the address of this node.
// This address is meant to be completely unique.
// This will return SDL_UNINITIALIZED if the SDL layer has not been initialized.
SdlStatus sdlAddress(SdlAddress *address);

//
// Communication.
//

// Does not block.
// Argument dataLength is in bytes.
// Will return SDL_UNINITIALIZED if the SDL has not been initialized yet.
SdlStatus sdlTransmit(SdlPacketType type,
                      SdlAddress destination,
                      uint8_t *data,
                      uint8_t dataLength);

// Does not block.
// Creates a deep copy of a packet to receive.
// If there is nothing to receive, this will return SDL_EMPTY.
// If the SDL layer is not initialized yet, this will return SDL_UNINITIALIZED.
SdlStatus sdlReceive(SdlPacket *packet);

// Do something kinda like a CCA.
// FIXME:
uint8_t sdlActivity(void);

// Whether or not to use CSMA mechanism per transmit.
extern uint8_t sdlCsmaOn;
#define SDL_CSMA_ON  (1)
#define SDL_CSMA_OFF (0)
#define sdlCsmaIsOn()   (sdlCsmaOn)
#define sdlCsmaSetOn()  (sdlCsmaOn = SDL_CSMA_ON)
#define sdlCsmaSetOff() (sdlCsmaOn = SDL_CSMA_OFF)

// CSMA parameters.
extern uint8_t sdlCsmaRetries;
#define SDL_CSMA_RETRIES 0
#define sdlCsmaSetRetries(r) (sdlCsmaRetries = r)
#define sdlCsmaGetRetries()  (sdlCsmaRetries)

//
// Utilities
//

// Takes an SdlPacket struct and converts it to a flat buffer.
void sdlPacketToFlatBuffer(SdlPacket *packet, uint8_t *buffer);

#endif /* __SDL_H__ */

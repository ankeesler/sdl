//
// sdl-types.h
//
// Andrew Keesler
//
// May 2, 2015
//
// SDL types.
//

#ifndef __SDL_TYPES_H__
#define __SDL_TYPES_H__

#include <stdint.h>
#include "sdl-protocol.h"

// A generic status code in SDL.
enum {
  // Success!
  SDL_SUCCESS       = 0x00,
  // The SDL layer does not have any received data in it.
  SDL_EMPTY         = 0x01,
  // The SDL layer is uninitialized.
  SDL_UNINITIALIZED = 0x02,
  // A fatal failure has occured.
  SDL_FATAL         = 0x03,
};
typedef uint8_t SdlStatus;

// The type of SDL packet.
enum {
  SDL_PACKET_TYPE_DATA = SDL_MAC_SDU_TYPE_DATA,
};
typedef uint8_t SdlPacketType;

// An SDL address.
typedef uint32_t SdlAddress;

// A packet from the point of view of the SDL layer.
typedef struct {
  SdlPacketType type;
  uint16_t sequence;

  SdlAddress source;
  SdlAddress destination;

  uint8_t data[SDL_PHY_SDU_MAX];
  uint8_t dataLength;
} SdlPacket;

#endif /* __SDL_TYPES_H__ */

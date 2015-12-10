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

#include "sdl-protocol.h"

#include <stdint.h>
#include <stdbool.h>

// A generic status code in SDL.
enum {
  /* General */
  // Success!
  SDL_SUCCESS  = 0x00,
  // A fatal failure has occured.
  SDL_FATAL    = 0x01,
  // There was an error with the size of a buffer.
  SDL_SIZE_ERR = 0x02,

  /* MAC */
  // The SDL MAC layer does not have any received data in it.
  SDL_MAC_EMPTY         = 0x11,
  // The SDL MAC layer is uninitialized.
  SDL_MAC_UNINITIALIZED = 0x12,

  /* PHY */
  // A transmit failure has occured.
  SDL_TRANSMIT_FAILURE = 0x21,

  /* SNET */
  // The node cannot be communicated with.
  SDL_SNET_COM_FAILURE           = 0x33,
};
typedef uint8_t SdlStatus;

// The type of SDL packet.
enum {
  SDL_PACKET_TYPE_DATA = SDL_MAC_SDU_TYPE_DATA,
};
typedef uint8_t SdlPacketType;

// An SDL address.
typedef uint32_t SdlAddress;

// A packet from the point of view of the MAC layer.
typedef struct {
  SdlPacketType type;
  uint16_t sequence;

  SdlAddress source;
  SdlAddress destination;

  uint8_t data[SDL_PHY_SDU_MAX];
  uint8_t dataLength;
} SdlPacket;

// A counter value.
enum {
  // How many times the MAC gets a packet for which it doesn't have room.
  SDL_COUNTER_MAC_RX_OVERFLOW = 0x01,
};
typedef uint8_t SdlCounter;

#endif /* __SDL_TYPES_H__ */

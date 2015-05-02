//
// sdl-protocol.h
//
// Andrew Keesler
//
// February 14, 2015
//
// SDL protocol definitions.
//

#ifndef __SDL_PROTOCOL_H__
#define __SDL_PROTOCOL_H__

// ----------------------------------------------------------------------------
// PHY

// Packet.
/*
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | PHY HEADER |                 PHY PAYLOAD                                  |
  | length     |                  payload                                     |
  | 1 byte     |                  variable                                    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

// Header size.
#define SDL_PHY_PDU_LENGTH 1

// Max payload.
#define SDL_PHY_SDU_MAX (0xFF - SDL_PHY_PDU_LENGTH)

// ----------------------------------------------------------------------------
// MAC

// Header (big endian).
/*
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  frame control                       |        sequence number             |
  |T T| RESERVERD                        |        uniquely identifies packet  |
  |(type:2)  2 bytes total               |        2 bytes                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                        source address                                     |
  |                        uniquely identifies originating node               |
  |                        4 bytes                                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                        destination address                                |
  |                        uniquely identifies destination node               |
  |                        4 bytes                                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                        data                                               |
  |                        payload                                            |
  |                        variable                                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                        ...                                                |
  |                        ...                                                |
  |                        ...                                                |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

// Header size.
#define SDL_MAC_PDU_LENGTH 12

// Max payload.
#define SDL_MAC_SDU_MAX (SDL_PHY_SDU_MAX - SDL_MAC_PDU_LENGTH)

// Types.
#define SDL_MAC_SDU_TYPE_DATA (0)

// Addresses.
#define SDL_MAC_ADDRESS_BROADCAST (0xFFFFFFFF)

#endif /* __SDL_PROTOCOL_H__ */

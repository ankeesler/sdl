//
// sdl-protocol.h
//
// Andrew Keesler
//
// February 14, 2015
//
// SDL protocol definitions.
//

// ----------------------------------------------------------------------------
// PHY

// Packet.
/*
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | PHY HEADER |                 PHY PAYLOAD                                  |
  | length     |                  payload                                     |
  | 1 byte     |                   variable                                   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

// Max payload.
#define SDL_MAX_PAYLOAD 254

// ----------------------------------------------------------------------------
// MAC

// Header.
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
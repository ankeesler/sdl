//
// sdl.h
//
// Andrew Keesler
//
// November 16, 2014
//
// Simulated data link layer interface.
//

#ifndef __SDL_H__
#define __SDL_H__

// Return values.
#define SDL_SUCCESS 0x00
#define SDL_ERROR_NETWORK_SATURATED 0x01
#define SDL_ERROR_COLLISION 0x02
#define SDL_ERROR_NETWORK_EMPTY 0x03

// This is the port that SDL uses.
#define SDL_PORT 8923

// A pseudo-baud rate.
#define SDL_BAUD 255

// RX timeout.
#define SDL_RCVTIMEO_S 1

// Network 'bandwidth' in packets.
#define SDL_BANDWIDTH 10

// Network packet lifetime in microseconds.
#define SDL_PACKET_LIFETIME_US 1000000 /* 1 second */

//
// User must define this. It is like the main loop. Bitches.
//
int SDL_USER_MAIN(void);

//
// Utilities.
//

// Does not block.
int sdlTransmit(unsigned char *data, int length);

// Does block.
int sdlReceive(unsigned char *buffer, int length);

int sdlGetNodeCount(void);

#endif /* __SDL_H__ */

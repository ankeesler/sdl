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

#ifndef __SDL_MAIN_C__
#define main SDL_USER_MAIN
#else
int SDL_USER_MAIN(int argc, char *argv[]);
#endif

// Return values.
#define SDL_SUCCESS 0x00
#define SDL_ERROR_NETWORK_SATURATED 0x01
#define SDL_ERROR_COLLISION 0x02
#define SDL_ERROR_NETWORK_EMPTY 0x03
#define SDL_STATUS_DATA_MTU_EXCEEDED 0x04
#define SDL_ERROR_NETWORK_UP 0x05

// Network 'bandwidth' in packets.
#define SDL_BANDWIDTH 10

// Network packet lifetime in microseconds.
#define SDL_PACKET_LIFETIME_US 1000000 /* 1 second */

// A MTU for the link...yeah its fake.
// In bytes.
#define SDL_MTU 255

// The maximum number of hosts for one network.
#define SDL_MAX_HOSTS 10

// CSMA.

// Whether or not to use CSMA mechanism per transmit.
extern int sdlCsmaOn;
#define SDL_CSMA_ON 1
#define sdlCsmaIsOn()   (sdlCsmaOn)
#define sdlCsmaSetOn()  (sdlCsmaOn = 1)
#define sdlCsmaSetOff() (sdlCsmaOn = 0)

// CSMA parameters.
extern int sdlCsmaRetries;
#define SDL_CSMA_RETRIES 3
#define sdlCsmaSetRetries(r) (sdlCsmaRetries = r)
#define sdlCsmaGetRetries()  (sdlCsmaRetries)

//
// Management.
//

// Pass a positive integer for a key. If it hasn't seen it yet,
// it will give you a new id. If it has seen it, it will
// give you the id that was given before.
// Returns -1 on failure. This can happen if a negative key is passed
// or the network is full.
int sdlInterfaceId(int key);

//
// Communication.
//

// Does not block.
// Length is in bytes.
int sdlTransmit(unsigned char *data, int length);

// Does block.
// Length is in bytes.
int sdlReceive(unsigned char *buffer, int length);

// Do something kinda like a CCA.
int sdlActivity(void);

#endif /* __SDL_H__ */

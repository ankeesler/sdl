//
// sdl-internal.h
//
// Andrew Keesler
//
// December 5, 2014
//
// Internal definitions for SDL.

#include <stdint.h>

int SDL_NETWORK_UP(void);
int SDL_NETWORK_DOWN(void);

// Reset the interface ids.
void sdlResetInterfaceIds(void);

// The radio has received length number of bytes.
void sdlRadioReceiveIsr(uint8_t *data, uint8_t length);

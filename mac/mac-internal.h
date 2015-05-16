//
// mac-internal.h
//
// Andrew Keesler
//
// December 5, 2014
//
// Internal definitions for the SDL MAC.

#include <stdint.h>

int SDL_NETWORK_UP(void);
int SDL_NETWORK_DOWN(void);

// Reset the interface ids.
void sdlResetInterfaceIds(void);
//
// sdl-internal.h
//
// Andrew Keesler
//
// December 5, 2014
//
// Internal definitions for SDL.

int SDL_NETWORK_UP(void);
int SDL_NETWORK_DOWN(void);

// Reset the interface ids.
void sdlResetInterfaceIds(void);

// ----------------------------------------------------------------------------
// Internal child API

// The radio has received length number of bytes.
// Called in an interupt context, i.e., when the child process gets
// a signal from the parent process.
// Returns 0 iff successfully read the PHY PDU + SDU.
int sdlRadioReceivedIsr(int fd);
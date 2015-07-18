//
// mac-internal.h
//
// Andrew Keesler
//
// December 5, 2014
//
// Internal definitions for the SDL MAC.

#include <signal.h> // sig_atomic_t

// SdlCounter value for MAC rx overflow.
extern volatile sig_atomic_t macRxOverflow;

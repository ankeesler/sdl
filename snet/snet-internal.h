//
// snet-internal.h
//
// Andrew Keesler
//
// February 13, 2015
//
// Internal defines for SNET.
//

#include <signal.h>

#define CHILD_QUIT_SIGNAL SIGUSR1
#define snetChildQuit() kill(getppid(), CHILD_QUIT_SIGNAL)

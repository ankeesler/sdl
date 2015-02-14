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

// Child -> parent saying that it is done running.
#define CHILD_QUIT_SIGNAL SIGUSR1
#define snetChildQuit() kill(getppid(), CHILD_QUIT_SIGNAL)

// We can use the same signal here since it is going the opposite
// direction as above.
#define CHILD_ALERT_SIGNAL SIGUSR1
#define snetParentAlert(pid) kill(pid, CHILD_ALERT_SIGNAL)
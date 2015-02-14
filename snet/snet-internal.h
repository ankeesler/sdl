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

// The signal that the parent uses to alert the child that there is
// something for them in the pipe.
#define CHILD_ALERT_SIGNAL SIGUSR1
#define snetChildAlert(pid) kill(pid, CHILD_ALERT_SIGNAL)

// The signal that the parent uses to tell the child to quit
// immediately.
#define CHILD_QUIT_SIGNAL SIGTERM
#define snetChildQuit(pid) kill(pid, CHILD_QUIT_SIGNAL)
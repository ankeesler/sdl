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

// The signal used to notify the parent that the child has stuff
// for it to read off of the file descriptor.
#define PARENT_ALERT_SIGNAL SIGUSR1
#define snetParentAlert() kill(getppid(), PARENT_ALERT_SIGNAL)

// The index of argv that the child node name lives in.
#define CHILD_NAME_INDEX (1)

// The index of argv that the parent to child fd lives in.
#define PARENT_TO_CHILD_FD_INDEX (2)

// The index of argv that the child to parent fd lives in.
#define CHILD_TO_PARENT_FD_INDEX (3)

// Internal child exit values.
#define CHILD_EXIT_SUCCESS    0x00
#define CHILD_EXIT_EXECL_FAIL 0x01

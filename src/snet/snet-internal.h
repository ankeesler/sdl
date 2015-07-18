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
// For the phy-test, we send this signal to ourselves so that we
// can make sure it was sent. If this was not the case, then we would
// have to deal with sending the PARENT_ALERT_SIGNAL to make or the
// terminal, which would result in a failing test.
#define PARENT_ALERT_SIGNAL SIGUSR1
#ifdef PHY_TEST
  #define snetParentAlert() kill(getpid(), PARENT_ALERT_SIGNAL)
#else
  #define snetParentAlert() kill(getppid(), PARENT_ALERT_SIGNAL)
#endif

// The child uses this signal to tell the parent that it is ready
// to go. Same thing as above here - for phy-test, send ourself
// this signal to test for it.
#define CHILD_READY_SIGNAL SIGUSR2
#ifdef PHY_TEST
  #define childReady() kill(getpid(), CHILD_READY_SIGNAL);
#else
  #define childReady() kill(getppid(), CHILD_READY_SIGNAL);
#endif

// The index of argv that the child node name lives in.
#define CHILD_NAME_INDEX (1)

// The index of argv that the parent to child fd lives in.
#define PARENT_TO_CHILD_FD_INDEX (2)

// The index of argv that the child to parent fd lives in.
#define CHILD_TO_PARENT_FD_INDEX (3)

// Internal child exit values.
#define CHILD_EXIT_SUCCESS     0x00
#define CHILD_EXIT_EXECL_FAIL  0x01
#define CHILD_EXIT_BAD_READ    0x02
#define CHILD_EXIT_BAD_COMMAND 0x03

// Child to parent commands.
#define CHILD_TO_PARENT_COMMAND_TRANSMIT (0x01)
#define CHILD_TO_PARENT_COMMAND_UART     (0x02)

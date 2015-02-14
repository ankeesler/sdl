//
// snet-main.c
//
// Andrew Keesler
//
// February 13, 2015
//
// Main code for node on a SNET.
//

#define __SNET_C__
#include "snet.h"
#include "snet-internal.h"

#include <stdio.h>
#include <unit-test.h>

// The reading fd for the pipe.
static int fd;

static void cleanup(void)
{
  // Close the read end of the pipe.
  close(fd);
}

static void signalHandler(int signal)
{
  if (signal == CHILD_ALERT_SIGNAL) {
    // Read the command out of the pipe.
    int command, readSize;
    if ((readSize = read(fd, &command, sizeof(command))) != sizeof(command))
      printf("(readSize:%d)", readSize); // TODO: report error.

    // Handle the command, remembering that this is
    // called in an interupt context.
    switch (command) {
    case NOOP:
      printf("(noop:%d)", getpid());
      break;
    default:
      printf("(command:%d)", command); // TODO: report error.
    }
  } else if (signal == CHILD_QUIT_SIGNAL) {
    cleanup();
    exit(0);
  }
}

// Get an int from a buffer, as described in snet.c.
// This is not endian-safe!!!
static void dumpLowNibbles(unsigned char buf[], int *n)
{
  int i;
  for (*n = i = 0; i < (sizeof(int) * 2); i ++)
    *n |= (0x0F & buf[i]);
}

int main(int argc, char *argv[])
{
  int ret;

  // The first argument past the program name is pipe fds from the
  // parent.
  if (argc < 2) {
    return 1;
  } else {
    dumpLowNibbles((unsigned char *)argv[1], &fd);
  }

  // Say that we want to handle the CHILD_ALERT_SIGNAL signal.
  // This will be our parent telling us that there is data for
  // us in the pipe.
  signal(CHILD_ALERT_SIGNAL, signalHandler);
  
  // Also say that we want to handle the CHILD_QUIT_SIGNAL,
  // for when our parent wants us to quit immediately.
  signal(CHILD_QUIT_SIGNAL, signalHandler);

  // Call the child node's main function.
  ret = SNET_MAIN(argc, argv);

  // When done, cleanup.
  cleanup();

  exit(ret);
}
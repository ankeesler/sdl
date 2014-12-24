//
// sdl-main.c
//
// Andrew Keesler
//
// December 5, 2014
//
// Main code for SDL.

#define __SDL_MAIN_C__

#include <stdio.h>
#include <stdlib.h>

#include "sdl.h"
#include "sdl-log.h"
#include "sdl-internal.h"

#define USAGE "sdl -n nodeCount"
#define PRINT_USAGE() printf("Usage: %s\n", USAGE);

#define SOFT_ASSERT_OPTION_ARG_COUNT(opt)                           \
  printf("Error: Incorrect argument count for option %s.\n", #opt); \
  PRINT_USAGE();                                                    \
  return 1;

static int nodeCount = 0;
int sdlGetNodeCount(void) { return nodeCount; }

int main(int argc, char *argv[])
{
  int err;

  // Get options.
  if (0 /* argc == 1 */) {
    PRINT_USAGE();
    return 0;
  } else {
    int i = 1;
    while (i < argc) {
      // Starting from the top of this loop assumes an option -[a-z].
      switch (argv[i][1]) {
      case 'n':
        if (i+1 >= argc) {
          // TODO: we don't really need this...right?
          //SOFT_ASSERT_OPTION_ARG_COUNT(n);
        }
        nodeCount = atoi(argv[++i]);
      default:
        ; // pass
      }
      i ++;
    }
  }

  // Validate the options.
  if (0 /*nodeCount <= 0*/) {
    printf("Error: Node count must be at least 1.\n");
    return 2;
  }

  // Try to start the network.
  if ((err = SDL_NETWORK_UP())) {
    printf("Error: Network could not be brought up. (err : %d)\n", err);
    return err;
  }

  // Maybe start the log. This will be macro'd out if it is not used.
  if ((err = sdlLogInit()))
    printf("Error: Log could not be started. (err : %d)\n", err);

  // Let the user do their thing.
  if ((err = SDL_USER_MAIN(argc, argv)))
    printf("Error: SDL_USER_MAIN() returned error.\n");

  // Maybe dump the log. This will be macro'd out if it is not used.
  if ((err = sdlLogDump()))
    printf("Error: Could not dump log. (err : %d)\n", err);

  // Quit out of the network.
  if ((err = SDL_NETWORK_DOWN()))
    printf("Error: Could not take network down. (err : %d)\n", err);

  return err;
}


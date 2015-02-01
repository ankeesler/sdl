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

int main(int argc, char *argv[])
{
  int err;

  // Try to start the network.
  if ((err = SDL_NETWORK_UP())) {
    printf("Error: Network could not be brought up. (err : %d)\n", err);
    return err;
  }
  
  // Reset the interface ids.
  sdlResetInterfaceIds();

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


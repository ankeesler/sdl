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

int main(int argc, char *argv[])
{
  // Call the child node's main function.
  SNET_MAIN(argc, argv);
  
  // When done, tell parent that you are done.
  snetChildQuit();

  return 0;
}

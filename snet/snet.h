//
// snet.h
//
// Andrew Keesler
//
// February 8, 2015
//
// Simulated network interface.
//

#ifndef __SNET_H__
#define __SNET_H__

#include <unistd.h>
#include <stdint.h>

#include "sdl-types.h"

#if !defined(__SNET_C__) && !defined(__SNET_TEST_C__)
#define main SNET_MAIN
#else
int SNET_MAIN(int argc, char *argv[]);
#endif

//
// Configuration.
//

#define SNET_MAX_HOSTS 10

//
// Management.
//

// Initialize the network.
// Also works for resetting the network.
void snetManagementInit(void);

// Deinitialize the network.
// Returns the number of nodes that were active in the network.
// MUST BE CALLED!!!
uint8_t snetManagementDeinit(void);

// Get the count of nodes in the network.
uint8_t snetManagementSize(void);

//
// Nodes.
//

// A node in the network.
typedef struct {
  const char *image;
  const char *name;
  int mask;
  pid_t pid;
  int fd;
} SnetNode;

// A command for a node.
enum {
  /* No operation */
  NOOP     = 0x01, // args: void

  /* Tell the PHY that there is incoming SDL data */
  RECEIVE  = 0x02, // args: uint8_t *bytes

  /* Tell the PHY to transmit SDL data. */
  TRANSMIT = 0x03, // args: uint8_t *bytes

  /* A command that should never be sent to a node. */
  NIL      = 0x04, // args: void
};
typedef uint8_t SnetNodeCommand;

// Create a node.
// The node is not yet added to the network.
// Returns NULL if the node could not be created.
SnetNode *snetNodeMake(const char *image, const char *name);

// Start a node with its image.
// A node can be turned off with snetNodeStop.
// If SDL_FATAL is returned, then there was a system process issue.
SdlStatus snetNodeStart(SnetNode *node);

// Cut the power to a node.
// In order to turn the node back on, snetNodeAdd should be called.
SdlStatus snetNodeStop(SnetNode *node);

// Command the node to do something.
SdlStatus snetNodeCommand(SnetNode *node, SnetNodeCommand command, ...);

#endif /* __SNET_H__ */

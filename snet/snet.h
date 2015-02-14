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

#if !defined(__SNET_C__) && !defined(__SNET_TEST_C__)
#define main SNET_MAIN
#else
int SNET_MAIN(int argc, char *argv[]);
#endif

// Status.

#define SNET_STATUS_SUCCESS 0
#define SNET_STATUS_UNKNOWN_NODE 1
#define SNET_STATUS_INVALID_NETWORK_STATE 2
#define SNET_STATUS_CANNOT_START_NODE 3
#define SNET_STATUS_CANNOT_COMMAND_NODE 4

// Configuration.

#define SNET_MAX_HOSTS 10

// PHY payload.
#define SNET_MAX_PAYLOAD 254

// Management.

// Initialize the network.
// Also works for resetting the network.
void snetManagementInit(void);

// Deinitialize the network.
// MUST BE CALLED!!!
void snetManagementDeinit(void);

// Nodes.

// A node in the network.
typedef struct {
  const char *image;
  const char *name;
  int mask;
  pid_t pid;
  int fd;
} SnetNode;

// A command for a node.
typedef enum {
  NOOP,     // args: void
  TRANSMIT, // args: int length, unsigned char *bytes
} SnetNodeCommand;

// Create a node.
// The node is not yet added to the network.
// Returns NULL if the node could not be created.
SnetNode *snetNodeMake(const char *image, const char *name);

// Add a node to the simulated network.
// Returns a SNET_STATUS_ definition.
int snetNodeAdd(SnetNode *node);

// Remove the node from the network.
// Returns a SNET_STATUS_ definition.
int snetNodeRemove(SnetNode *node);

// Get the count of nodes in the network.
int snetNodeCount(void);

// Command the node to do something.
int snetNodeCommand(SnetNode *node, SnetNodeCommand command, ...);

#endif /* __SNET_H__ */

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

#include "sdl-types.h"

#include <unistd.h>

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
  uint8_t mask;
  pid_t pid;
  int parentToChildFd, childToParentFd;
} SnetNode;

// A command for a node.
enum {
  /* No operation */
  NOOP     = 0x01, // args: void

  /* Tell the PHY that there is incoming SDL data */
  RECEIVE  = 0x02, // args: uint8_t *bytes

  /* Tell the PHY to transmit SDL data. */
  TRANSMIT = 0x03, // args: uint8_t *bytes

  /* Push a virtual button on a node. */
  BUTTON   = 0x04, // args: uint8_t button

  /* A command that should never be sent to a node. */
  NIL      = 0xFF, // args: void
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
// In order to turn the node back on, snetNodeStart should be called.
// Other than the image and the name, one should not cache any values
// of a SnetNode!
SdlStatus snetNodeStop(SnetNode *node);

// Command the node to do something.
SdlStatus snetNodeCommand(SnetNode *node, SnetNodeCommand command, ...);

// Set the parent-level pseudo-ISR for receiving UART bytes.
// If this is not set then there will be no notification for when a node
// sends bytes across its UART.
// This function returns the previous nodeUartIsr.
// The ISR takes the name of the node, the received data bytes, and the length
// of the received bytes for its parameters.
typedef void (*SnetNodeUartIsr)(const char *, uint8_t *, uint8_t);
SnetNodeUartIsr snetNodeUartIsr(SnetNodeUartIsr uartIsr);

#endif /* __SNET_H__ */

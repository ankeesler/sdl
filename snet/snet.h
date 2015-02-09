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

// Nodes.

// A node in the network.
typedef struct {
  const char *image;
  const char *name;
} SnetNode;

// Add a node to the simulated network.
void snetNodeAdd(SnetNode *node);

// Remove the node from the network.
void snetNodeRemove(SnetNode *node);

// Get the count of nodes in the network.
int snetNodeCount(void);

#endif /* __SNET_H__ */

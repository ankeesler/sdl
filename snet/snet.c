//
// snet.c
//
// Andrew Keesler
//
// February 8, 2015
//
// SNET implementations.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "snet.h"
#include "snet-internal.h"

// Prototypes.
void nodeRemoveReally(SnetNode *node);

// SnetNode mask.
/* is the node being used? */
#define SNET_NODE_MASK_USED (1 << 0) 
/* is the node on the network?? */
#define SNET_NODE_MASK_ON_NETWORK (1 << 1)

// Possible nodes.
static SnetNode nodePool[SNET_MAX_HOSTS];
#define nodeIsUnknown(node)                     \
  (!(node)                                      \
   || ((node) < nodePool)                       \
   || ((node) > (nodePool + SNET_MAX_HOSTS)))

// The number of nodes added to the network.
static int nodesInNetwork = 0;

// Signal handler for parent.
// When finished, the child process will send the parent the
// CHILD_QUIT_SIGNAL signal.

void signalHandler(int signal)
{
  int i;

  if (signal == CHILD_QUIT_SIGNAL) {
    // For each node, check which one says that it is on the network,
    // but the process is not running.
    for (i = 0; i < SNET_MAX_HOSTS; i ++) {
      if (nodePool[i].mask & SNET_NODE_MASK_ON_NETWORK
          && kill(0, nodePool[i].pid)) {
        nodeRemoveReally(nodePool + i);        
        break;
      }
    }
  }
}

void snetManagementInit(void)
{
  int i;
  for (i = 0; i < SNET_MAX_HOSTS; i ++)
    nodePool[i].mask = 0;
  nodesInNetwork = 0;

  // Set signal handler.
  signal(CHILD_QUIT_SIGNAL, signalHandler);
}

void snetManagementDeinit(void)
{
  int pid;
  while ((pid = wait(NULL)) != -1) ;
}

// Returns the next available node index, or -1 if there is none.
static int nextAvailableNode(void)
{
  int i;
  for (i = 0; i < SNET_MAX_HOSTS; i ++) {
    if (!(nodePool[i].mask & SNET_NODE_MASK_USED)) {
      return i;
    }
  }
  return -1;
}

SnetNode *snetNodeMake(const char *image, const char *name)
{
  SnetNode *node = NULL;
  int i = nextAvailableNode();

  if (i != -1) {
    node = nodePool + i;
    node->image = image;
    node->name = name;
    node->mask |= SNET_NODE_MASK_USED;
  }

  return node;
}

int snetNodeAdd(SnetNode *node)
{
  pid_t newPid;

  if (nodeIsUnknown(node))
    return SNET_STATUS_UNKNOWN_NODE;

  if (node->mask & SNET_NODE_MASK_ON_NETWORK)
    return SNET_STATUS_INVALID_NETWORK_STATE;

  node->mask |= SNET_NODE_MASK_ON_NETWORK;

  if ((newPid = fork()) == -1) {
    // Failure.
    return SNET_STATUS_CANNOT_START_NODE;
  } else if (newPid) {
    // Parent.
    node->pid = newPid;
  } else {
    // Child.
    execl(node->image, 0);
  }
  
  nodesInNetwork ++;

  return SNET_STATUS_SUCCESS;
}

int snetNodeRemove(SnetNode *node)
{
  if (nodeIsUnknown(node))
    return SNET_STATUS_UNKNOWN_NODE;

  if (!(node->mask & SNET_NODE_MASK_ON_NETWORK))
    return SNET_STATUS_INVALID_NETWORK_STATE;

  kill(node->pid, SIGKILL);
  nodeRemoveReally(node);

  return SNET_STATUS_SUCCESS;
}

void nodeRemoveReally(SnetNode *node)
{
  node->mask &= ~SNET_NODE_MASK_ON_NETWORK;
  nodesInNetwork --;
}

int snetNodeCount(void)
{
  return nodesInNetwork;
}

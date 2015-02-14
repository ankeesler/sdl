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
#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <unit-test.h>

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

// If we copy an int to an argument list, the argument
// list might think it ends sooner than it does because
// of once of the 4 int bytes being 0. So, store a 4-byte int
// in the lower 8 nibbles of a byte buffer, and then null
// terminate the buffer.
// This is bad because it assumes an int is 4 bytes!!!
static void fillLowNibbles(unsigned char buf[], int n)
{
  buf[0] = 0xF0 | ((0xF0000000 & n) >> 0x1C);
  buf[1] = 0xF0 | ((0x0F000000 & n) >> 0x18);
  buf[2] = 0xF0 | ((0x00F00000 & n) >> 0x14);
  buf[3] = 0xF0 | ((0x000F0000 & n) >> 0x10);
  buf[4] = 0xF0 | ((0x0000F000 & n) >> 0x0C);
  buf[5] = 0xF0 | ((0x00000F00 & n) >> 0x08);
  buf[6] = 0xF0 | ((0x000000F0 & n) >> 0x04);
  buf[7] = 0xF0 | ((0x0000000F & n) >> 0x00);
  
  buf[8] = 0; // null terminator
}

int snetNodeAdd(SnetNode *node)
{
  pid_t newPid;
  int fd[2];
  unsigned char buf[(2 * sizeof(int)) + 1];

  if (nodeIsUnknown(node))
    return SNET_STATUS_UNKNOWN_NODE;

  if (node->mask & SNET_NODE_MASK_ON_NETWORK)
    return SNET_STATUS_INVALID_NETWORK_STATE;

  if (pipe(fd) || (newPid = fork()) == -1) {
    // Failure.
    return SNET_STATUS_CANNOT_START_NODE;
  } else if (newPid) {
    // Parent.
    close(fd[0]); // close the read end of the pipe
    node->pid = newPid;
    node->fd = fd[1];
  } else {
    // Child.
    close(fd[1]); // close the write end of the pipe
    fillLowNibbles(buf, fd[0]);
    execl(node->image, node->image, buf, 0);

    // If execl returns, then this is bad.
    exit(1);
  }
  
  node->mask |= SNET_NODE_MASK_ON_NETWORK;
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

int snetNodeCommand(SnetNode *node, SnetNodeCommand command, ...)
{
  int i;
  va_list args;
  
  if (nodeIsUnknown(node))
    return SNET_STATUS_UNKNOWN_NODE;

  if (!(node->mask & SNET_NODE_MASK_ON_NETWORK))
    return SNET_STATUS_INVALID_NETWORK_STATE;

  // First write the command to the pipe.
  if (write(node->fd, &command, sizeof(command) != sizeof(command)))
    return SNET_STATUS_CANNOT_COMMAND_NODE;

  // Now, write the arguments to the command.
  va_start(args, command);
  switch (command) {
  case NOOP:
    break;
  case TRANSMIT: {
    int length;
    unsigned char buf[SNET_MAX_PAYLOAD];
    
    // Length.
    length = va_arg(args, int);
    write(node->fd, &length, sizeof(int));
    
    // Payload.
    for (i = 0; i < length; i ++)
      buf[i] = va_arg(args, int); // why can't this be unsigned char?
    write(node->fd, buf, length);
  }
    break;
  default:
    ; // TODO: report error.
  }
  va_end(args);

  // Finally, try tell the child that they have something coming for them.
  return (snetParentAlert(node->pid)
          ? SNET_STATUS_CANNOT_COMMAND_NODE
          : SNET_STATUS_SUCCESS);
}

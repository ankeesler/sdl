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
#include <errno.h>
#include <stdint.h>

#include "snet.h"
#include "snet-internal.h"
#include "snet-debug.h"
#include "sdl-types.h"

#include "sdl-protocol.h"

// ----------------------------------------------------------------------------
// Prototypes.

static void nodeRemoveReally(SnetNode *node);
static SnetNode *findNodeForPid(pid_t pid);
#define processIsAlive(pid) (kill(pid, 0) == 0)

// ----------------------------------------------------------------------------
// Definitions.

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
static uint8_t nodesInNetwork = 0;

// ----------------------------------------------------------------------------
// Testing.

#ifdef SNET_TEST

  #include <sys/time.h> // gettimeofday()

  typedef struct {
    pid_t pid;
    const char *nodeName;
    struct timeval time;
    int signal;
  } SignalData;
  
  #define SIGNAL_DATA_SIZE (32)
  static SignalData signalData[SIGNAL_DATA_SIZE];
  static uint8_t signalDataIndex = 0;

  static void logSignalData(pid_t pid, int signal)
  {
    SnetNode *node;

    if (signalDataIndex < SIGNAL_DATA_SIZE) {
      node = findNodeForPid(pid);
      signalData[signalDataIndex].pid = pid;
      signalData[signalDataIndex].signal = signal;
      signalData[signalDataIndex].nodeName = (node ? node->name : "???");
      gettimeofday(&(signalData[signalDataIndex].time), NULL);
      signalDataIndex ++;
    }
  }
  
  void printSignalData(void)
  {
    uint8_t i;

    printf("\n");
    for (i = 0; i < signalDataIndex; i ++) {
      printf("(signalData[%d] = {pid=%d(%s), time=%ld us, signal=%d(%s)})\n",
             i,
             signalData[i].pid,
             signalData[i].nodeName,
             ((signalData[i].time.tv_sec * 1000000)
               + signalData[i].time.tv_usec),
             signalData[i].signal,
             signalNames[signalData[i].signal]);
    }
  }  
#else
  #define logSignalData(...)
  void printSignalData(void) {}
#endif

// ----------------------------------------------------------------------------
// Management.

static SnetNode *findNodeForPid(pid_t pid)
{
  uint8_t i;
  for (i = 0; i < SNET_MAX_HOSTS; i ++) {
    if (nodePool[i].pid == pid) {
      return nodePool + i;
    }
  }
  return NULL;
}

void signalHandler(int signal)
{
  pid_t pid = 0;

  if (signal == SIGCHLD) {
    // Wait to see which child process is quitting.
    pid = wait(NULL);

    // Find that child process and really remove it.
    nodeRemoveReally(findNodeForPid(pid));
  } else if (signal == PARENT_ALERT_SIGNAL) {
    // Uh, which node was this from?
    /*
    // For each node that is on (except for this one), tell them to receive it.
    for (i = 0; i < SNET_MAX_HOSTS && status == SDL_SUCCESS; i ++) {
      if (nodePool[i].mask & SNET_NODE_MASK_ON_NETWORK
          && nodePool[i].pid != node->pid) {
        status = snetNodeCommand(&nodePool[i], RECEIVE, data);
      }
    }
    */
  }

  logSignalData(pid, signal);
}

void snetManagementInit(void)
{
  int i;
  for (i = 0; i < SNET_MAX_HOSTS; i ++)
    nodePool[i].mask = 0;
  nodesInNetwork = 0;

  // Signal handler for child processes that die.
  signal(SIGCHLD, signalHandler);

  // Signal handler for child process to communicate with parent.
  signal(PARENT_ALERT_SIGNAL, signalHandler);
}

uint8_t snetManagementDeinit(void)
{
  uint8_t count = 0, i = 0;

  while (i < SNET_MAX_HOSTS) {
    if ((nodePool[i].mask & SNET_NODE_MASK_USED)
        && (snetNodeStop(nodePool + i) == SDL_SUCCESS)) {
      count ++;
    }
    i ++;
  }

  return count;
}
  
uint8_t snetManagementSize(void)
{
  return nodesInNetwork;
}

// Returns the next available node index, or -1 if there is none.
static uint8_t nextAvailableNode(void)
{
  uint8_t i;
  for (i = 0; i < SNET_MAX_HOSTS; i ++) {
    if (!(nodePool[i].mask & SNET_NODE_MASK_USED)) {
      return i;
    }
  }
  return -1;
}

// ----------------------------------------------------------------------------
// Nodes.

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

SdlStatus snetNodeStart(SnetNode *node)
{
  pid_t newPid;
  int fd[2];
  char fdBuf[10]; // max digits of signed 32-bit integer

  if (nodeIsUnknown(node))
    return SDL_SNET_UNKNOWN_NODE;

  if (node->mask & SNET_NODE_MASK_ON_NETWORK)
    return SDL_SNET_INVALID_NETWORK_STATE;

  if (pipe(fd) || (newPid = fork()) == -1) {
    // Failure.
    return SDL_FATAL;
  } else if (newPid) {
    // Parent.
    close(fd[0]); // close the read end of the pipe
    node->pid = newPid;
    node->fd = fd[1];
  } else {
    // Child.
    close(fd[1]); // close the write end of the pipe
    sprintf(fdBuf, "%d", fd[0]);
    execl(node->image, node->image, node->name, fdBuf, 0);

    // If execl returns, then this is bad.
    exit(1);
  }
  
  node->mask |= SNET_NODE_MASK_ON_NETWORK;
  nodesInNetwork ++;

  return SDL_SUCCESS;
}

SdlStatus snetNodeStop(SnetNode *node)
{
  if (nodeIsUnknown(node))
    return SDL_SNET_UNKNOWN_NODE;

  if (!(node->mask & SNET_NODE_MASK_ON_NETWORK))
    return SDL_SNET_INVALID_NETWORK_STATE;

  kill(node->pid, SIGTERM);
  nodeRemoveReally(node);

  return SDL_SUCCESS;
}

static void nodeRemoveReally(SnetNode *node)
{
  // For safety.
  if (!node)
    return;
    
  // If this bit has already been turned off, then assume that
  // this stuff has been taken care of.
  if (!(node->mask & SNET_NODE_MASK_ON_NETWORK))
    return;

  // Turn off the bit in the node's mask and drop the number of
  // nodes in a network.
  node->mask &= ~SNET_NODE_MASK_ON_NETWORK;
  nodesInNetwork --;

  // Close the pipe.
  close(node->fd);

  // Wait on the process.
  waitpid(node->pid, NULL, 0);
}

// ----------------------------------------------------------------------------
// Commands.

SdlStatus snetNodeCommand(SnetNode *node, SnetNodeCommand command, ...)
{
  uint8_t *data;
  va_list args;
  SdlStatus status = SDL_SUCCESS;
  
  if (nodeIsUnknown(node))
    return SDL_SNET_UNKNOWN_NODE;

  if (!(node->mask & SNET_NODE_MASK_ON_NETWORK))
    return SDL_SNET_INVALID_NETWORK_STATE;

  // First write the command to the pipe.
  if (write(node->fd, &command, sizeof(command)) != sizeof(command))
    return SDL_SNET_COM_FAILURE;

  // Now, write the arguments to the command.
  va_start(args, command);
  switch (command) {
  case NOOP:
    status = SDL_SUCCESS;
    break;
  case TRANSMIT:
  case RECEIVE:
    // Get the pointer to the raw SDL packet.
    // The first byte is the length of the whole packet.
    data = va_arg(args, void *);
    status = (write(node->fd, data, data[0]) == data[0]
              ? SDL_SUCCESS
              : SDL_SNET_COM_FAILURE);
    break;
  default:
    status = SDL_SNET_UNKNOWN_COMMAND;
  }
  va_end(args);

  // Finally, try tell the child that they have something coming for them.
  return (status == SDL_SUCCESS
          ? (snetChildAlert(node->pid)
             ? SDL_SNET_COM_FAILURE
             : SDL_SUCCESS)
          : status);
}

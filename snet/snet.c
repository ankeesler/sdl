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
#include <signal.h>
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
#define SNET_NODE_MASK_USED       (0x01)
/* is the node on the network?? */
#define SNET_NODE_MASK_ON_NETWORK (0x02)

// Possible nodes.
static SnetNode nodePool[SNET_MAX_HOSTS];
#define nodeIsUnknown(node)                     \
  (!(node)                                      \
   || ((node) < nodePool)                       \
   || ((node) > (nodePool + SNET_MAX_HOSTS)))

// The number of nodes added to the network.
static uint8_t nodesInNetwork = 0;

static volatile sig_atomic_t newestChildReady = 0;
static volatile sig_atomic_t waitForNewChild  = 1;

static SnetNodeUartIsr uartIsr = NULL;

// ----------------------------------------------------------------------------
// Testing.

#ifdef SNET_TEST

  #include <sys/time.h> // gettimeofday()

  typedef struct {
    pid_t pid;
    const char *nodeName;
    struct timeval time;
    int signal;
    int ret;
  } SignalData;
  
  #define SIGNAL_DATA_SIZE (32)
  static SignalData signalData[SIGNAL_DATA_SIZE];
  static uint8_t signalDataIndex = 0;

  static void logSignalData(pid_t pid, int signal, int ret)
  {
    SnetNode *node;

    if (signalDataIndex < SIGNAL_DATA_SIZE) {
      node = findNodeForPid(pid);
      signalData[signalDataIndex].pid = pid;
      signalData[signalDataIndex].signal = signal;
      signalData[signalDataIndex].nodeName = (node ? node->name : "???");
      signalData[signalDataIndex].ret = ret;
      gettimeofday(&(signalData[signalDataIndex].time), NULL);
      signalDataIndex ++;
    }
  }
  
  void printSignalData(void)
  {
    uint8_t i;

    printf("(signal count = %d)\n", signalDataIndex);
    for (i = 0; i < signalDataIndex; i ++) {
      printf("(signalData[%d] = {pid=%d(%s), time=%ld us, signal=%d(%s), ret=%d})\n",
             i,
             signalData[i].pid,
             signalData[i].nodeName,
             ((signalData[i].time.tv_sec * 1000000)
               + signalData[i].time.tv_usec),
             signalData[i].signal,
             signalNames[signalData[i].signal],
             signalData[i].ret);
    }
    fflush(stdout);
  }

  void printNodeData(void)
  {
    uint8_t i;
    for (i = 0; i < SNET_MAX_HOSTS; i ++ ) {
      printf("i = %d, image = %s, name = %s, mask = 0x%02X, pid = %d\n",
             i, nodePool[i].image, nodePool[i].name, nodePool[i].mask, nodePool[i].pid);
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

void signalHandler(int signal, siginfo_t *info, void *wut)
{
  pid_t pid = 0;
  int stat = 0;
  uint8_t i, buffer[UINT8_MAX], command, length;
  SdlStatus status = SDL_SUCCESS;
  SnetNode *node = NULL;

  if (signal == SIGCHLD) {
    // Wait to see which child process is quitting.
    pid = wait(&stat);

    // Find that child process and really remove it.
    if ((node = findNodeForPid(pid))) {
      nodeRemoveReally(node);
    }
  } else if (signal == PARENT_ALERT_SIGNAL) {
    if ((node = findNodeForPid((pid = info->si_pid)))) {
      // Read command from child.
      // TODO: die on bad read?
      read(node->childToParentFd, &command, sizeof(command));
      if (command == CHILD_TO_PARENT_COMMAND_TRANSMIT) {
        // Read packet from buffer.
        // TODO: die on bad read?
        read(node->childToParentFd, buffer + 0, sizeof(uint8_t));
        read(node->childToParentFd, buffer + 1, buffer[0] - 1);
        // For each node that is on (except for this one), tell them to
        // receive the packet.
        for (i = 0; i < SNET_MAX_HOSTS && status == SDL_SUCCESS; i ++) {
          if (nodePool[i].mask & SNET_NODE_MASK_ON_NETWORK
              && nodePool[i].pid != pid) {
            status = snetNodeCommand(&nodePool[i], RECEIVE, buffer);
          }
        }
      } else if (command == CHILD_TO_PARENT_COMMAND_UART) {
        if (uartIsr) {
          // Cheat and use the TRANSMIT buffer to read in the data.
          // TODO: die on bad read?
          read(node->childToParentFd, &length, sizeof(uint8_t));
          read(node->childToParentFd, buffer, length);
          uartIsr(node->name, buffer, length);
        }
      }
    }
  } else if (signal == CHILD_READY_SIGNAL) {
    pid = info->si_pid;
    if (!newestChildReady) {
      newestChildReady = pid;
    }
  } else if (signal == SIGALRM) {
    waitForNewChild = 0;
  }

  logSignalData(pid, signal, WEXITSTATUS(stat));
}

void snetManagementInit(void)
{
  uint8_t i;
  struct sigaction action = {
    .sa_sigaction = signalHandler,
    .sa_flags     = SA_SIGINFO,
  };
  sigemptyset(&action.sa_mask);

  for (i = 0; i < SNET_MAX_HOSTS; i ++)
    nodePool[i].mask = 0;
  nodesInNetwork = 0;

  // Signal handler for child processes that die.
  sigaction(SIGCHLD, &action, NULL); // oact - don't care

  // Signal handler for child process to communicate with parent.
  sigaction(PARENT_ALERT_SIGNAL, &action, NULL); // oact - don't care

  // Signal handler for when our child is ready.
  sigaction(CHILD_READY_SIGNAL, &action, NULL); // oact - don't care

  // Signal handler for SIGALRM.
  sigaction(SIGALRM, &action, NULL); // oact - don't care
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
    node->mask = SNET_NODE_MASK_USED;
  }

  return node;
}

SdlStatus snetNodeStart(SnetNode *node)
{
  SdlStatus status = SDL_SUCCESS;
  pid_t newPid;
  int parentToChildPipe[2], childToParentPipe[2];

  // max digits of signed 32-bit integer
  char parentToChildFdBuf[10], childToParentFdBuf[10]; 

  if (nodeIsUnknown(node))
    return SDL_SNET_UNKNOWN_NODE;

  if (node->mask & SNET_NODE_MASK_ON_NETWORK)
    return SDL_SNET_INVALID_NETWORK_STATE;

  if (pipe(parentToChildPipe)
      || pipe(childToParentPipe)
      || (newPid = fork()) == -1) {
    // Failure.
    status = SDL_FATAL;
  } else if (newPid) {
    // Parent.
    close(parentToChildPipe[0]); // close the read end of the pipe
    close(childToParentPipe[1]); // close the write end of the pipe
    node->pid = newPid;
    node->parentToChildFd = parentToChildPipe[1];
    node->childToParentFd = childToParentPipe[0];

    // Wait a second for the child to tell us that it is ready.
    alarm(1);
    while (waitForNewChild && newestChildReady != newPid) ;
    if (!waitForNewChild) {
      status = SDL_SNET_COM_FAILURE;
    }
    newestChildReady = 0;
    waitForNewChild = 1;
  } else {
    // Child.
    close(parentToChildPipe[1]); // close the write end of the pipe
    close(childToParentPipe[0]); // close the read end of the pipe
    sprintf(parentToChildFdBuf, "%d", parentToChildPipe[0]);
    sprintf(childToParentFdBuf, "%d", childToParentPipe[1]);
    execl(node->image,
          node->image,
          node->name,
          parentToChildFdBuf,
          childToParentFdBuf,
          0);

    // If execl returns, then this is bad.
    exit(CHILD_EXIT_EXECL_FAIL);
  }
  
  node->mask |= SNET_NODE_MASK_ON_NETWORK;
  nodesInNetwork ++;

  return status;
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

  // Close the pipes.
  close(node->parentToChildFd);
  close(node->childToParentFd);

  // Wait on the process.
  waitpid(node->pid, NULL, 0);
}

// ----------------------------------------------------------------------------
// Commands.

SdlStatus snetNodeCommand(SnetNode *node, SnetNodeCommand command, ...)
{
  va_list args;
  SdlStatus status = SDL_SUCCESS;
  
  if (nodeIsUnknown(node))
    return SDL_SNET_UNKNOWN_NODE;

  if (!(node->mask & SNET_NODE_MASK_ON_NETWORK))
    return SDL_SNET_INVALID_NETWORK_STATE;

  // First write the command to the pipe.
  if (write(node->parentToChildFd,
            &command,
            sizeof(command))
      != sizeof(command))
    return SDL_SNET_COM_FAILURE;

  // Now, write the arguments to the command.
  va_start(args, command);
  switch (command) {
  case NOOP:
    status = SDL_SUCCESS;
    break;
  case TRANSMIT:
  case RECEIVE: {
    // Get the pointer to the raw SDL packet.
    // The first byte is the length of the whole packet.
    uint8_t *data = va_arg(args, void *);
    status = (write(node->parentToChildFd, data, data[0]) == data[0]
              ? SDL_SUCCESS
              : SDL_SNET_COM_FAILURE);
    break;
  }
  case BUTTON: {
    int buttonArg = (int)va_arg(args, void *);
    uint8_t button = (uint8_t)buttonArg;
    status = ((write(node->parentToChildFd, &button, sizeof(button))
               == sizeof(button))
              ? SDL_SUCCESS
              : SDL_SNET_COM_FAILURE);
    break;
  }
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

SnetNodeUartIsr snetNodeUartIsr(SnetNodeUartIsr newUartIsr)
{
  SnetNodeUartIsr oldUartIsr = uartIsr;
  uartIsr = newUartIsr;
  return oldUartIsr;
}

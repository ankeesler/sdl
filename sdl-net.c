//
// sdl.c
//
// Andrew Keesler
//
// November 16, 2014
//
// Simulated data link layer.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

#include "sdl.h"
#include "sdl-log.h"

typedef struct {
  unsigned char data[SDL_MTU];
  struct timeval birth;
} QData;

static int packetIsAlive(QData *data, struct timeval *now);  

static QData networkQ[SDL_BANDWIDTH];
static int head, tail;
#define NETWORK_Q_IS_EMPTY()   (head == tail)
#define NETWORK_Q_IS_FULL()    (head == ((tail+1)%SDL_BANDWIDTH))
#define NETWORK_Q_INCREMENT(n) (n = (n == SDL_BANDWIDTH-1 ? 0 : n+1))

// TODO: combine these masks.
// Network state.
// I am going to pretend that these are atomic ops, even though they are not...
static int maskNet = 0;
#define NET_MASK_INIT()     (maskNet = 0)

#define NET_MASK_UP         (0x01)
#define NET_MASK_SET_UP()   (maskNet |= NET_MASK_UP)
#define NET_MASK_READ_UP()  (maskNet & NET_MASK_UP)
#define NET_MASK_SET_DOWN() (maskNet &= ~NET_MASK_UP)

#define NET_MASK_TX         (0x02)
#define NET_MASK_SET_TX()   (maskNet |= NET_MASK_TX)
#define NET_MASK_READ_TX()  (maskNet & NET_MASK_TX)
#define NET_MASK_CLEAR_TX() (maskNet &= ~NET_MASK_TX)

#define NET_MASK_RX         (0x04)
#define NET_MASK_SET_RX()   (maskNet |= NET_MASK_RX)
#define NET_MASK_READ_RX()  (maskNet & NET_MASK_RX)
#define NET_MASK_CLEAR_RX() (maskNet &= ~NET_MASK_RX)

#define NET_MASK_Q          (0x08)
#define NET_MASK_SET_Q()    (maskNet |= NET_MASK_Q)
#define NET_MASK_READ_Q()   (maskNet & NET_MASK_Q)
#define NET_MASK_CLEAR_Q()  (maskNet &= ~NET_MASK_Q)
#define NET_MASK_LOCK_Q()   (NET_MASK_SET_Q())
#define NET_MASK_UNLOCK_Q() (NET_MASK_CLEAR_Q())

// CSMA stuff.
int sdlCsmaOn      = SDL_CSMA_ON;
int sdlCsmaRetries = SDL_CSMA_RETRIES;
int randMask    = 0x00000007;

// FD task stuff.
static pthread_t fdThread;
#define CHILD_TASK_DUTY_CYCLE_US 5000

static void *fdTask(void *)
{
  while (NET_MASK_READ_UP()) {
    // Sleep so we are not hammering the socket.
    usleep(CHILD_TASK_DUTY_CYCLE_US);
    
    // Lock the file from writing.
    
    // If we reach EOF, goto unlock.
    
    // Try to read one byte from the socket.
    // This is the length.
    
    // Read the next byte. That is the inteface id.
    // TODO: should this be a byte? We return an int from sdlInterfaceId().
    // If this is our interface id, then goto unlock.
    
    // Read the rest of the packet. If we get EOF, goto unlock.
    
    while (NET_MASK_READ_Q()) ; // Wait for queue to unlock.
    NET_MASK_LOCK_Q();
    // Write the packet to the networkQ.
    NET_MASK_UNLOCK_Q();
    
    // Done with the file, so goto unlock.
    
    // unlock:
    //   Unlock the file from writing and reading.
  }
  
  pthread_exit(NULL);
}

int SDL_NETWORK_UP(void)
{
  pid_t pid;
  
  // Initialize the network.
  NET_MASK_INIT();
  head = tail = 0;
  
  // Go ahead and bring the network up so that the child task
  // can start spinning.
  NET_MASK_SET_UP();
  
  // Create the thread to read from the file. We need a thread
  // and not a process so that we can share the masks and the network Q.
  return (pthread_create(&fdThread, NULL, fdTask, NULL)
          ? SDL_ERROR_NETWORK_UP
          : SDL_SUCCESS);
}

int SDL_NETWORK_DOWN(void)
{
  int returnStatus;
  
  // Bring the network down.
  NET_MASK_SET_DOWN();
  
  // Wait for the fd thread to quit.
  return pthread_join(fdThread);
}

static int transmit(unsigned char *data,
                    int length,
                    int timeoutUS,
                    int retries)
{
  // If someone else (including ourselves) is already
  // transmitting, then the CSMA mechanism may fix that.
  // If we are transmitting, then we will not be able to transmit
  // even with CSMA, and that sounds about right.
  
  // If the network is full, return that we are saturated.
  if (NETWORK_Q_IS_FULL())
    return SDL_ERROR_NETWORK_SATURATED;
  
  // Try file lock.
  
  // If we fail and CSMA is enabled, do that.
  // Else, fail completely.
  if (sdlCsmaIsOn() && retries) {
    usleep(timeoutUS);
    return transmit(data,
                    length,
                    ((1 << (randMask & rand())) - 1),
                    -- retries);
  } else {
    return SDL_ERROR_COLLISION;
  }
  
  // Lock the file.
  
  // Write to the file.
  
  // Unlock the file.
  
  // Log. Will be stubbed out if client does not want to use it.
  sdlLogTx(data, length);

  return SDL_SUCCESS;  
}

int sdlTransmit(unsigned char *data, int length)
{
  return transmit(data, length, 0, 0);
}

int sdlReceive(unsigned char *buffer, int length)
{
  int status = SDL_ERROR_NETWORK_EMPTY;
  struct timeval now;
  
  // If we are already receiving, then we can't receive more.
  if (NET_MASK_READ_RX())
    return SDL_ERROR_COLLISION;
  
  while (NET_MASK_READ_Q()) ; // Wait for the network Q to unlock.
  NET_MASK_LOCK_Q();
  
  // Get the current time here. If we had to wait for the network Q to
  // unlock, isn't that part of the recieving process?
  gettimeofday(&now);
  
  // Find the next packet that is alive.
  do {
    if (packetIsAlive(networkQ + head, &now)) {
      // Deep copy.
      memcpy(data, networkQ[head].data, length);
      status = SDL_SUCCESS;
    }
    NETWORK_Q_INCREMENT(head);
  } while (!NETWORK_Q_IS_EMPTY()
           && status == SDL_ERROR_NETWORK_EMPTY);
  
  // Unlock the network Q again.
  NET_MASK_UNLOCK_Q();
  
  return status;
}

static int packetIsAlive(QData *data, struct timeval *now)
{
  long diffS;
  long diffUS;

  // Screw rollovers.
  // Ok ok ok just for now.
  diffS = now->tv_sec - data->birth.tv_sec;
  diffUS = ((now->tv_usec - data->birth.tv_usec)
            + (diffS * 1000000)); // microseconds per second
  return (diffUS < SDL_PACKET_LIFETIME_US);
}

int sdlActivity(void)
{
  return (head > tail ? head - tail : tail - head);
}


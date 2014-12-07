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

#include "sdl.h"

typedef struct {
  unsigned char *data;
  struct timeval birth;
} QData;

static int packetIsAlive(QData *data, struct timeval *now);  

static QData networkQ[SDL_BANDWIDTH];
static int head, tail;
#define NETWORK_Q_IS_EMPTY()   (head == tail)
#define NETWORK_Q_IS_FULL()    (head == ((tail+1)%SDL_BANDWIDTH))
#define NETWORK_Q_INCREMENT(n) (n = (n == SDL_BANDWIDTH-1 ? 0 : n+1))

static int txHO = 0, rxHO = 0;

int SDL_NETWORK_UP(void)
{
  head = tail = txHO = rxHO = 0;
  return 0;
}

int SDL_NETWORK_DOWN(void)
{
  return 0;
}

int sdlTransmit(unsigned char *data, int length)
{
  // If someone else (including ourselves) is already
  // transmitting, then we will just cheat and call it a collision.
  // We do not currently use any CSMA mechanism.
  if (txHO)
    return SDL_ERROR_COLLISION;
  else if (NETWORK_Q_IS_FULL())
    return SDL_ERROR_NETWORK_SATURATED;
  else
    txHO = rxHO = 1;

  // Deep copy.
  networkQ[tail].data
    = (unsigned char *)malloc(sizeof(unsigned char) * length);
  memcpy(networkQ[tail].data, data, length);
  gettimeofday(&(networkQ[tail].birth), NULL);

  NETWORK_Q_INCREMENT(tail);

  txHO = rxHO = 0;

  return SDL_SUCCESS;
}

int sdlReceive(unsigned char *buffer, int length)
{
  while (rxHO)
    ; // block
  
  // This is dangerous AF.
  if (NETWORK_Q_IS_EMPTY())
    return SDL_ERROR_NETWORK_EMPTY;

  // Don't rx right now because someone could steal data
  // that was meant for us.
  // Don't tx right now because...just don't! 
  rxHO = txHO = 1;

  // Deep copy. Find the first packet that is still alive.
  do {
    struct timeval now;
    gettimeofday(&now, NULL);
    if (packetIsAlive(networkQ + head, &now)) {
      memcpy(buffer, networkQ[head].data, length);
      free(networkQ[head].data);
      NETWORK_Q_INCREMENT(head);
      txHO = rxHO = 0;
      return SDL_SUCCESS;
    }
    NETWORK_Q_INCREMENT(head);
  } while (!NETWORK_Q_IS_EMPTY());
  
  rxHO = txHO = 0;
  return SDL_ERROR_NETWORK_EMPTY;
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

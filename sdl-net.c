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
#include "unistd.h"

#include "mac.h"
#include "sdl-log.h"
#include "mac-protocol.h"

typedef struct {
  unsigned char data[SDL_PHY_SDU_MAX];
  struct timeval birth;
} QData;

static int packetIsAlive(QData *data, struct timeval *now);  

static QData networkQ[SDL_BANDWIDTH];
static int head, tail;
#define NETWORK_Q_IS_EMPTY()   (head == tail)
#define NETWORK_Q_IS_FULL()    (head == ((tail+1)%SDL_BANDWIDTH))
#define NETWORK_Q_INCREMENT(n) (n = (n == SDL_BANDWIDTH-1 ? 0 : n+1))

// This is basically our mutex for the networkQ.
volatile int maskHO;
#define HO_MASK_INIT()      HO_MASK_CLEAR_ALL()
#define HO_MASK_READ_TX()   (maskHO & 0x01) /* hardcode these so we can get atomic ops */
#define HO_MASK_SET_TX()    maskHO |= 0x01  /* hardcode these so we can get atomic ops */
#define HO_MASK_CLEAR_TX()  maskHO &= 0xFE  /* hardcode these so we can get atomic ops */
#define HO_MASK_READ_RX()   (maskHO & 0x02) /* hardcode these so we can get atomic ops */
#define HO_MASK_SET_RX()    maskHO |= 0x02  /* hardcode these so we can get atomic ops */
#define HO_MASK_CLEAR_RX()  maskHO &= 0xFD  /* hardcode these so we can get atomic ops */
#define HO_MASK_SET_ALL()   maskHO = 0x03  /* hardcode these so we can get atomic ops */
#define HO_MASK_CLEAR_ALL() maskHO = 0x00  /* hardcode these so we can get atomic ops */

// CSMA stuff.
int sdlCsmaOn      = SDL_CSMA_ON;
int sdlCsmaRetries = SDL_CSMA_RETRIES;
int randMask    = 0x00000007;

int SDL_NETWORK_UP(void)
{
  HO_MASK_INIT();
  return (head = tail = 0);
}

int SDL_NETWORK_DOWN(void)
{
  return 0;
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
  if (HO_MASK_READ_TX()) {
    if (sdlCsmaIsOn() && retries) {
      usleep(timeoutUS);
      return transmit(data,
                      length,
                      ((1 << (randMask & rand())) - 1),
                      -- retries);
    } else {
      return SDL_ERROR_COLLISION;
    }
  } else if (NETWORK_Q_IS_FULL()) {
    return SDL_ERROR_NETWORK_SATURATED;
  } else {
    HO_MASK_SET_ALL();
  }

  // Deep copy.
  memcpy(networkQ[tail].data, data, length);
  gettimeofday(&(networkQ[tail].birth), NULL);
  
  // Log. Will be stubbed out if client does not want to use it.
  sdlLogTx(data, length);

  NETWORK_Q_INCREMENT(tail);

  HO_MASK_CLEAR_ALL();

  return SDL_SUCCESS;  
}

int sdlTransmit(unsigned char *data, int length)
{
  return transmit(data, length, 0, 0);
}

int sdlReceive(unsigned char *buffer, int length)
{
  while (HO_MASK_READ_RX())
    ; // block
  
  // This is dangerous AF.
  if (NETWORK_Q_IS_EMPTY())
    return SDL_ERROR_NETWORK_EMPTY;

  // Don't rx right now because someone could steal data
  // that was meant for us.
  // Don't tx right now because...just don't! 
  HO_MASK_SET_ALL();

  // Deep copy. Find the first packet that is still alive.
  do {
    struct timeval now;
    gettimeofday(&now, NULL);
    if (packetIsAlive(networkQ + head, &now)) {
      memcpy(buffer, networkQ[head].data, length);
      NETWORK_Q_INCREMENT(head);
      
      // Log. Will be stubbed out if client does not want to use it.
      sdlLogRx(buffer, length);

      HO_MASK_CLEAR_ALL();

      return SDL_SUCCESS;
    }
    NETWORK_Q_INCREMENT(head);
  } while (!NETWORK_Q_IS_EMPTY());
  
  HO_MASK_CLEAR_ALL();
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

int sdlActivity(void)
{
  return (head > tail ? head - tail : tail - head);
}

// ----------------------------------------------------------------------------
// Internal child API

// TODO: there is work to be done to make this a safe ISR!!!
int sdlRadioReceivedIsr(int fd)
{
  int length;
  
  // See sdl-internal.h for definition of PHY PDU.
  
  // LOCK.
  HO_MASK_SET_ALL();
  
  // Only proceed to read out of the radio buffer if the network
  // Q has room for data.
  if (!NETWORK_Q_IS_FULL()) {
    // Read the length.
    if (read(fd, &length, sizeof(length)) != length)
      return 1;
  
    // Read the PHY payload into the next networkQ slot.
    if (read(fd, networkQ + tail, length) != length)
      return 1;
    
    // Set the birth of the packet.
    gettimeofday(&(networkQ[tail].birth), NULL);
  
    // Move the tail of the Q back one.
    NETWORK_Q_INCREMENT(tail);
  }
    
  // UNLOCK.
  HO_MASK_CLEAR_ALL();
  
  return 0;
}

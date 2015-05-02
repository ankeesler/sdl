//
// sdl.c
//
// Andrew Keesler
//
// May 2, 2015
//
// SDL implementations.
//

#include "sdl.h"
#include "sdl-internal.h"

#include <string.h> // memcpy()

// -----------------------------------------------------------------------------
// Declarations

static void communicationInit(void);

// -----------------------------------------------------------------------------
// Utilities

#define HIGH_BYTE(v) (uint8_t)((v & 0xFF00) >> 0x08)
#define LOW_BYTE(v)  (uint8_t)((v & 0x00FF) >> 0x00)

// Big-endian.
static void putAddressInBuffer(SdlAddress address, uint8_t *buffer)
{
  buffer[0] = (address & 0xFF000000) >> 0x18;
  buffer[1] = (address & 0x00FF0000) >> 0x10;
  buffer[2] = (address & 0x0000FF00) >> 0x08;
  buffer[3] = (address & 0x000000FF) >> 0x00;
}
static void getAddressFromBuffer(SdlAddress *address, uint8_t *buffer)
{
  *address = ((buffer[0] << 0x18)
              | (buffer[1] << 0x10)
              | (buffer[2] << 0x08)
              | (buffer[3] << 0x00));
}

// -----------------------------------------------------------------------------
// Management

#define STATE_INITIALIZED (0x01)
static uint32_t state = 0;

static SdlAddress ourAddress;

static uint16_t sequence;

SdlStatus sdlInit(SdlAddress address)
{
  ourAddress = address;

  communicationInit();

  state |= STATE_INITIALIZED;

  return SDL_SUCCESS;
}

SdlStatus sdlAddress(SdlAddress *address)
{
  if (!(state & STATE_INITIALIZED)) return SDL_UNINITIALIZED;

  *address = ourAddress;

  return SDL_SUCCESS;
}

// -----------------------------------------------------------------------------
// Communication

#define TX_BUFFER_SIZE SDL_PHY_SDU_MAX
static uint8_t txBuffer[TX_BUFFER_SIZE];

#define RX_BUFFER_SIZE 5
static SdlPacket rxBuffer[RX_BUFFER_SIZE];
static uint8_t rxHead, rxTail;
#define nextRxBufferIndex(i)                    \
  ((i+1) == RX_BUFFER_SIZE ? 0 : (i+1))
#define rxBufferIsFull()                        \
  (nextRxBufferIndex(rxTail) == rxHead)
#define rxBufferIsEmpty()                       \
  (rxHead == rxTail)

static void communicationInit(void)
{ 
  sequence = 0;
  rxHead = rxTail = 0;
}

SdlStatus sdlTransmit(SdlPacketType type,
                      SdlAddress destination,
                      uint8_t *data,
                      uint8_t dataLength)
{
  if (!(state & STATE_INITIALIZED)) return SDL_UNINITIALIZED;

  // Big endian (see sdl-protocol.h).
  
  // Frame control.
  txBuffer[0] |= type;

  // Sequence number.
  txBuffer[2] = HIGH_BYTE(sequence);
  txBuffer[3] = LOW_BYTE(sequence);

  // Source address.
  putAddressInBuffer(ourAddress, txBuffer + 4);
  
  // Destination address.
  putAddressInBuffer(destination, txBuffer + 8);

  // Data.
  memcpy(txBuffer + 12, data, dataLength);

  // If this is for us, put it in the receive queue.
  // TODO: this is agressive to call the ISR...but it is OK, right?
  if (destination == ourAddress) {
    sdlRadioReceiveIsr(txBuffer, SDL_MAC_PDU_LENGTH + dataLength);
  } else {
    // TODO: hand it to the radio.
  }

  return SDL_SUCCESS;
}
                    
SdlStatus sdlReceive(SdlPacket *packet)
{
  if (!(state & STATE_INITIALIZED)) return SDL_UNINITIALIZED;

  if (rxBufferIsEmpty()) return SDL_EMPTY;

  *packet = rxBuffer[rxHead];
  rxHead = nextRxBufferIndex(rxHead);
  
  return SDL_SUCCESS;
}

void sdlRadioReceiveIsr(uint8_t *data, uint8_t length)
{
  uint8_t dataLength = length - SDL_MAC_PDU_LENGTH;

  // If the queue is full, then we can't receive anymore.
  // TODO: report this.
  if (rxBufferIsFull()) return;

  // Big endian (see sdl-protocol.h).

  // Filter this packet immediately if it is not for us.
  getAddressFromBuffer(&(rxBuffer[rxTail].destination), data + 8);
  if (rxBuffer[rxTail].destination != ourAddress)
    return;
  
  // Frame control.
  rxBuffer[rxTail].type = data[0];

  // Sequence number.
  rxBuffer[rxTail].sequence = (data[2] << 0x08) | (data[3] << 0x00);

  // Source address.
  getAddressFromBuffer(&(rxBuffer[rxTail].source), data + 4);

  // Data.
  memcpy(rxBuffer[rxTail].data, data + 12, dataLength);

  rxTail = nextRxBufferIndex(rxTail);
}

uint8_t sdlActivity(void)
{
  return 0;
}

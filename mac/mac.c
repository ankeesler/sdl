//
// mac.c
//
// Andrew Keesler
//
// May 2, 2015
//
// SDL MAC layer.
//

#include "mac.h"
#include "phy.h"
#include "mac-internal.h"

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

void sdlPacketToFlatBuffer(SdlPacket *packet, uint8_t *buffer)
{
  // Big endian (see sdl-protocol.h).
  
  // Frame control.
  buffer[0] = packet->type;

  // Sequence number.
  buffer[2] = HIGH_BYTE(packet->sequence);
  buffer[3] = LOW_BYTE(packet->sequence);

  // Source address.
  putAddressInBuffer(packet->source, buffer + 4);
  
  // Destination address.
  putAddressInBuffer(packet->destination, buffer + 8);

  // Data.
  memcpy(buffer + 12, packet->data, packet->dataLength);
}

// -----------------------------------------------------------------------------
// Management

#define STATE_INITIALIZED (0x01)
static uint32_t state = 0;

static SdlAddress ourAddress;

SdlStatus sdlMacInit(SdlAddress address)
{
  ourAddress = address;

  communicationInit();

  state |= STATE_INITIALIZED;

  return SDL_SUCCESS;
}

SdlStatus sdlMacAddress(SdlAddress *address)
{
  if (!(state & STATE_INITIALIZED)) return SDL_UNINITIALIZED;

  *address = ourAddress;

  return SDL_SUCCESS;
}

// -----------------------------------------------------------------------------
// Communication

uint8_t sdlCsmaOn = SDL_CSMA_OFF;
uint8_t sdlCsmaRetries = SDL_CSMA_RETRIES;

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

static uint16_t sequence;

static void communicationInit(void)
{ 
  sequence = 0;
  rxHead = rxTail = 0;
}

SdlStatus sdlMacTransmit(SdlPacketType type,
                         SdlAddress destination,
                         uint8_t *data,
                         uint8_t dataLength)
{
  SdlPacket packet;

  if (!(state & STATE_INITIALIZED)) return SDL_UNINITIALIZED;

  packet.type = type;
  packet.sequence = sequence;
  packet.source = ourAddress;
  packet.destination = destination;
  memcpy(packet.data, data, dataLength);
  packet.dataLength = dataLength;

  sdlPacketToFlatBuffer(&packet, txBuffer);

  // If this is for us, put it in the receive queue.
  // This is agressive to call the ISR...but it is OK, right.
  if (destination == ourAddress) {
    sdlPhyReceiveIsr(txBuffer, SDL_MAC_PDU_LENGTH + dataLength);
  } else {
    // TODO: hand it to the phy.
  }

  return SDL_SUCCESS;
}
                    
SdlStatus sdlMacReceive(SdlPacket *packet)
{
  if (!(state & STATE_INITIALIZED)) return SDL_UNINITIALIZED;

  if (rxBufferIsEmpty()) return SDL_EMPTY;

  // Deep copy.
  *packet = rxBuffer[rxHead];
  rxHead = nextRxBufferIndex(rxHead);
  
  return SDL_SUCCESS;
}

void sdlPhyReceiveIsr(uint8_t *data, uint8_t length)
{
  uint8_t dataLength = length - SDL_MAC_PDU_LENGTH;

  // If the queue is full, then we can't receive anymore.
  // TODO: report this.
  if (rxBufferIsFull()) return;

  // Big endian (see sdl-protocol.h).

  // Filter this packet immediately if it is not for us.
  getAddressFromBuffer(&(rxBuffer[rxTail].destination), data + 8);
  if (rxBuffer[rxTail].destination != ourAddress
      && rxBuffer[rxTail].destination != SDL_MAC_ADDRESS_BROADCAST)
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

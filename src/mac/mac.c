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
#include "phy/phy-internal.h"

#include <string.h> // memcpy()
#include <signal.h> // sig_atomic_t

// -----------------------------------------------------------------------------
// Declarations

static void communicationInit(void);

// -----------------------------------------------------------------------------
// Utilities

// Big-endian.
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

SdlStatus sdlMacInit(SdlAddress address)
{
  ourAddress = address;

  communicationInit();

  state |= STATE_INITIALIZED;

  return SDL_SUCCESS;
}

SdlStatus sdlMacAddress(SdlAddress *address)
{
  if (!(state & STATE_INITIALIZED)) return SDL_MAC_UNINITIALIZED;

  *address = ourAddress;

  return SDL_SUCCESS;
}

// -----------------------------------------------------------------------------
// Communication

// TODO: make me psuedo-interrupt safe!

volatile sig_atomic_t macRxOverflow = 0;

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
  SdlStatus status = SDL_SUCCESS;

  if (!(state & STATE_INITIALIZED)) return SDL_MAC_UNINITIALIZED;

  packet.type = type;
  packet.sequence = sequence;
  packet.source = ourAddress;
  packet.destination = destination;
  memcpy(packet.data, data, dataLength);
  packet.dataLength = dataLength;

  sdlPacketToFlatBuffer(&packet, txBuffer);

  // If this is for us, put it in the receive queue.
  // Otherwise, throw it to the PHY.
  if (destination == ourAddress || destination == SDL_MAC_ADDRESS_BROADCAST) {
    phyReceiveIsr(txBuffer, SDL_MAC_PDU_LEN + dataLength);
  }
  if (destination != ourAddress) {
    status = sdlPhyTransmit(txBuffer, SDL_MAC_PDU_LEN + dataLength);
  }

  return status;
}
                    
SdlStatus sdlMacReceive(SdlPacket *packet)
{
  if (!(state & STATE_INITIALIZED)) return SDL_MAC_UNINITIALIZED;

  if (rxBufferIsEmpty()) return SDL_MAC_EMPTY;

  // Deep copy.
  *packet = rxBuffer[rxHead];
  rxHead = nextRxBufferIndex(rxHead);
  
  return SDL_SUCCESS;
}

// The data points to the first byte past the PHY PDU.
// The length is the length of the data vector.
void phyReceiveIsr(uint8_t *data, uint8_t length)
{
  uint8_t dataLength = length - SDL_MAC_PDU_LEN;

  // If the queue is full, then we can't receive anymore.
  if (rxBufferIsFull()) {
    if (macRxOverflow + 1) {
      macRxOverflow ++;
    }
    return;
  }

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

  // Data length.
  rxBuffer[rxTail].dataLength = dataLength;

  rxTail = nextRxBufferIndex(rxTail);
}

uint8_t sdlActivity(void)
{
  return 0;
}

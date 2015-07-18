//
// mac-util.c
//
// Andrew Keesler
//
// July 9, 2015
//
// SDL MAC layer utilities.
//

#include "mac.h"

#include <string.h> // memcpy()
#include <signal.h> // sig_atomic_t

#include "mac-internal.h"

// -----------------------------------------------------------------------------
// Private 

// Big-endian.
static void putAddressInBuffer(SdlAddress address, uint8_t *buffer)
{
  buffer[0] = (address & 0xFF000000) >> 0x18;
  buffer[1] = (address & 0x00FF0000) >> 0x10;
  buffer[2] = (address & 0x0000FF00) >> 0x08;
  buffer[3] = (address & 0x000000FF) >> 0x00;
}

// -----------------------------------------------------------------------------
// Public

void sdlPacketToFlatBuffer(SdlPacket *packet, uint8_t *buffer)
{
  // Big endian (see sdl-protocol.h).
  
  // Frame control.
  buffer[0] = packet->type;

  // Sequence number.
  buffer[2] = ((packet->sequence & 0xFF00) >> 0x08); // high byte
  buffer[3] = ((packet->sequence & 0x00FF) >> 0x00); // low byte

  // Source address.
  putAddressInBuffer(packet->source, buffer + 4);
  
  // Destination address.
  putAddressInBuffer(packet->destination, buffer + 8);

  // Data.
  memcpy(buffer + 12, packet->data, packet->dataLength);
}

bool sdlCounterValue(SdlCounter counter, uint32_t *value)
{
  bool legitCounterValue = true;

  switch (counter) {
  case SDL_COUNTER_MAC_RX_OVERFLOW:
    *value = macRxOverflow;
    break;
  default:
    legitCounterValue = false;
  }

  return legitCounterValue;
}

bool sdlCounterClear(SdlCounter counter)
{
  bool legitCounterValue = true;

  switch (counter) {
  case SDL_COUNTER_MAC_RX_OVERFLOW:
    macRxOverflow = 0;
    break;
  default:
    legitCounterValue = false;
  }

  return legitCounterValue;
}

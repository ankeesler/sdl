//
// sdl-test.c
//
// Andrew Keesler
//
// May 2, 2015
//
// SDL test.
//

#include <unit-test.h>

#define __SDL_MAIN_C__
#include "phy.h"
#include "mac.h"
#include "mac-internal.h"

#include <unistd.h> // getpid()

static SdlAddress source;
static SdlAddress destination = 0x00000000;

#define dataBufferLength (10)
static uint8_t data[dataBufferLength];

static SdlPacket packet;

int sanityCheck(void)
{
  // We shouldn't be able to check out ID, transmit, or receive
  // if we have not initialized.
  expect(sdlMacAddress(&source)
         == SDL_UNINITIALIZED);
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA, destination, data, dataBufferLength)
         == SDL_UNINITIALIZED);
  expect(sdlMacReceive(&packet)
         == SDL_UNINITIALIZED);

  // Initialization should go fine.
  expect(sdlMacInit(getpid())
         == SDL_SUCCESS);

  // Now we can get our address.
  expect(sdlMacAddress(&source)
         == SDL_SUCCESS);
  expect(source == getpid());
  
  // Should be able to transmit.
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA, destination, data, dataBufferLength)
         == SDL_SUCCESS);

  // Receiving something right now should return SDL_EMPTY.
  expect(sdlMacReceive(&packet)
         == SDL_EMPTY);

  return 0;
}

int loopbackTest(void)
{
  expect(sdlMacAddress(&source)
         == SDL_SUCCESS);

  // If we send something to ourself...
  expect(sdlMacTransmit(SDL_PACKET_TYPE_DATA, source, data, dataBufferLength)
         == SDL_SUCCESS);

  // ...then we should be able to receive it...right?
  expect(sdlMacReceive(&packet)
         == SDL_SUCCESS);
  expect(packet.type == SDL_PACKET_TYPE_DATA);
  expect(packet.source == source);
  expect(packet.destination == source);
  expect(!memcmp(packet.data, data, dataBufferLength));

  // After, the SDL should be empty.
  expect(sdlMacReceive(&packet)
         == SDL_EMPTY);

  return 0;
}

int broadcastTest(void)
{
  uint8_t flatPacket[255];

  // Destination address of SDL_MAC_ADDRESS_BROADCAST.
  flatPacket[8] = 0xFF;
  flatPacket[9] = 0xFF;
  flatPacket[10] = 0xFF;
  flatPacket[11] = 0xFF;
  
  // If we receive something broadcasted...
  sdlPhyReceiveIsr(flatPacket, SDL_MAC_PDU_LENGTH);

  // ...then we should be able to receive it.
  expect(sdlMacReceive(&packet)
         == SDL_SUCCESS);

  // After, the SDL should be empty.
  expect(sdlMacReceive(&packet)
         == SDL_EMPTY);

  // Destination address of almost SDL_MAC_ADDRESS_BROADCAST.
  flatPacket[8] = 0xF1;
  flatPacket[9] = 0xF2;
  flatPacket[10] = 0xF3;
  flatPacket[11] = 0xF4;
  sdlPhyReceiveIsr(flatPacket, SDL_MAC_PDU_LENGTH);
  expect(sdlMacReceive(&packet)
         == SDL_EMPTY);

  return 0;
}

int utilitiesTest(void)
{
  uint8_t flatPacket[SDL_MAC_SDU_MAX];

  packet.type = SDL_PACKET_TYPE_DATA;
  packet.sequence = 0xABCD;
  packet.source = 0x01234567;
  packet.destination = 0x89ABCDEF;
  memcpy(packet.data, data, dataBufferLength);
  packet.dataLength = dataBufferLength;

  sdlPacketToFlatBuffer(&packet, flatPacket);

  expect(flatPacket[0] == SDL_PACKET_TYPE_DATA);
  expect(flatPacket[2] == 0xAB);
  expect(flatPacket[3] == 0xCD);

  expect(flatPacket[4] == 0x01);
  expect(flatPacket[5] == 0x23);
  expect(flatPacket[6] == 0x45);
  expect(flatPacket[7] == 0x67);

  expect(flatPacket[8] == 0x89);
  expect(flatPacket[9] == 0xAB);
  expect(flatPacket[10] == 0xCD);
  expect(flatPacket[11] == 0xEF);

  expect(!memcmp(flatPacket + 12, data, dataBufferLength));

  return 0;
}

int main(void)
{
  run(sanityCheck);

  run(loopbackTest);
  run(broadcastTest);

  run(utilitiesTest);

  return 0;
}


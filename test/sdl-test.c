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
#include "sdl.h"
#include "sdl-internal.h"

#include <unistd.h> // getpid()

static SdlAddress source;
static SdlAddress destination = 0x00000000;

#define dataLength (10)
static uint8_t data[dataLength];

static SdlPacket packet;

int sanityCheck(void)
{
  // We shouldn't be able to check out ID, transmit, or receive
  // if we have not initialized.
  expect(sdlAddress(&source)
         == SDL_UNINITIALIZED);
  expect(sdlTransmit(SDL_PACKET_TYPE_DATA, destination, data, dataLength)
         == SDL_UNINITIALIZED);
  expect(sdlReceive(&packet)
         == SDL_UNINITIALIZED);

  // Initialization should go fine.
  expect(sdlInit(getpid())
         == SDL_SUCCESS);

  // Now we can get our address.
  expect(sdlAddress(&source)
         == SDL_SUCCESS);
  expect(source == getpid());
  
  // Should be able to transmit.
  expect(sdlTransmit(SDL_PACKET_TYPE_DATA, destination, data, dataLength)
         == SDL_SUCCESS);

  // Receiving something right now should return SDL_EMPTY.
  expect(sdlReceive(&packet)
         == SDL_EMPTY);

  return 0;
}

int loopbackTest(void)
{
  expect(sdlAddress(&source)
         == SDL_SUCCESS);

  // If we send something to ourself...
  expect(sdlTransmit(SDL_PACKET_TYPE_DATA, source, data, dataLength)
         == SDL_SUCCESS);

  // ...then we should be able to receive it...right?
  expect(sdlReceive(&packet)
         == SDL_SUCCESS);
  expect(packet.type == SDL_PACKET_TYPE_DATA);
  expect(packet.source == source);
  expect(packet.destination == source);
  expect(!memcmp(packet.data, data, dataLength));

  // After, the SDL should be empty.
  expect(sdlReceive(&packet)
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
  sdlRadioReceiveIsr(flatPacket, SDL_MAC_PDU_LENGTH);

  // ...then we should be able to receive it.
  expect(sdlReceive(&packet)
         == SDL_SUCCESS);

  // After, the SDL should be empty.
  expect(sdlReceive(&packet)
         == SDL_EMPTY);

  // Destination address of almost SDL_MAC_ADDRESS_BROADCAST.
  flatPacket[8] = 0xF1;
  flatPacket[9] = 0xF2;
  flatPacket[10] = 0xF3;
  flatPacket[11] = 0xF4;
  sdlRadioReceiveIsr(flatPacket, SDL_MAC_PDU_LENGTH);
  expect(sdlReceive(&packet)
         == SDL_EMPTY);

  return 0;
}

int main(void)
{
  run(sanityCheck);
  run(loopbackTest);
  run(broadcastTest);
  return 0;
}


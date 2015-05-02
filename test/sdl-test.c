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

int main(void)
{
  run(sanityCheck);
  run(loopbackTest);
  return 0;
}


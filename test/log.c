//
// log.c
//
// Andrew Keesler
//
// December 13, 2014
//
// Logging test.

#include <unit-test.h>
#include <string.h>

#include "mac.h"
#include "sdl-log.h"

int logOnTest(void)
{
  expect(sdlLogOn());
  return 0;
}

int logStuffTest(void)
{
  unsigned char shortBuffer[] = {0x01, 0x02, 0x03, 0x04};
  unsigned char shortBufferLength = 4;
  unsigned char longBuffer[] = {1,2,4,8,16,32,64,128,255};
  unsigned char longBufferLength = 9;

  unsigned char data[10];

  // Send something.
  expect(!sdlTransmit(shortBuffer, shortBufferLength));

  // Receive something.
  expect(!sdlReceive(data, shortBufferLength));
  expect(!memcmp(data, shortBuffer, shortBufferLength));

  // Send two things.
  expect(!sdlTransmit(longBuffer, longBufferLength));
  expect(!sdlTransmit(shortBuffer, shortBufferLength));

  // Receive two things.
  expect(!sdlReceive(data, longBufferLength));
  expect(!memcmp(data, longBuffer, longBufferLength));
  expect(!sdlReceive(data, shortBufferLength));
  expect(!memcmp(data, shortBuffer, shortBufferLength));

  // Send two things.
  expect(!sdlTransmit(shortBuffer, shortBufferLength));
  expect(!sdlTransmit(longBuffer, longBufferLength));

  // Receive one thing.
  expect(!sdlReceive(data, shortBufferLength));
  expect(!memcmp(data, shortBuffer, shortBufferLength));
  
  // Send one thing.
  expect(!sdlTransmit(shortBuffer, shortBufferLength));

  // Receive two things.
  expect(!sdlReceive(data, longBufferLength));
  expect(!memcmp(data, longBuffer, longBufferLength));
  expect(!sdlReceive(data, shortBufferLength));
  expect(!memcmp(data, shortBuffer, shortBufferLength));

  return 0;
}

int SDL_USER_MAIN(void)
{
  announce();
  run(logOnTest);
  run(logStuffTest);
  return 0;
}

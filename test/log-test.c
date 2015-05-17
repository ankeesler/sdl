//
// log-test.c
//
// Andrew Keesler
//
// May 17, 2015
//
// Unit test for logging.
//

#include <unit-test.h>
#include <stdint.h>

#include "snet.h"
#include "phy.h"

// TODO: remove me when child process dumps to the log itself.
#include "cap/sdl-log.h"

// Stub.
void sdlPhyReceiveIsr(uint8_t *data, uint8_t length) {}

static uint8_t data[0xFF];
#define SMALL_DATA_SIZE (10)
#define LARGE_DATA_SIZE (25)

int writeBytes(void)
{
  uint8_t i;
  
  for (i = 0; i < SMALL_DATA_SIZE; i ++) data[i] = i;
  expect(sdlPhyTransmit(data, SMALL_DATA_SIZE) == SDL_SUCCESS);
  sdlPhyReceiveIsr(data, SMALL_DATA_SIZE);
  
  for (i = 0; i < LARGE_DATA_SIZE; i ++) data[i] = LARGE_DATA_SIZE - i;
  expect(sdlPhyTransmit(data, LARGE_DATA_SIZE) == SDL_SUCCESS);
  sdlPhyReceiveIsr(data, LARGE_DATA_SIZE);

  return 0;
}

int grepTest(void)
{
  // TODO: remove this thing and let the child process call it when it exits.
  sdlLogDump();

  // Sanity check.
  expect(!system("grep -q CAPTURE " SDL_LOG_FILE));
  expect( system("grep -q TUNA    " SDL_LOG_FILE));
  
  // TX.
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) TX \\[0x0B, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) TX \\[0x1A, 0x19, 0x18, .*, 0x10, 0x0F, 0x0E, .*\\]' "                       SDL_LOG_FILE));
  
  // RX. TODO:
  
  return 0;
}

int main(void)
{
  announce();

  run(writeBytes);
  
  run(grepTest);
  
  return 0;
}

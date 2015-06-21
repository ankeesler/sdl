//
// phy-test.c
//
// Andrew Keesler
//
// May 22, 2015
//
// Test for PHY layer APIs.
// This test is really from the child's perspective.
//

#include <unit-test.h>
#include <signal.h>   // kill
#include <assert.h>   // assert
#include <string.h>   // memcpy

#include "snet.h"
#include "snet-internal.h" // snetChildAlert
#include "sdl-types.h"
#include "sdl-protocol.h"

// Keep this inline with PHY_TEST_IN in the makefile.
// This test reads from this file for the PARENT_TO_CHILD data.
#define PHY_TEST_IN_NAME "phy-test.in"

// -----------------------------------------------------------------------------
// Stubs

int sdlLogDump(void) { return 0; }
int sdlLogTx(unsigned char *bytes, int length) { return 0; }
int sdlLogRx(unsigned char *bytes, int length) { return 0; }

static uint8_t phyReceiveData[SDL_PHY_SDU_MAX];
static uint8_t phyReceiveDataLength = 0;
void sdlPhyReceiveIsr(uint8_t *data, uint8_t count)
{
  memcpy(phyReceiveData, data, count);
  phyReceiveDataLength = count;
}

// -----------------------------------------------------------------------------
// Utility

static const uint8_t phyTestInData[] = {
  // sanityTest
  NOOP, // First command is a NOOP command, i.e., do nothing.

  // receiveTest
  RECEIVE, // SnetNodeCommand
  0x02   , // SDL_PHY_PDU
  0x80   , // SDL_PHY_SDU

  // transmitTest
  TRANSMIT, // SnetNodeCommand
  0x02    , // SDL_PHY_PDU
  0xA0    , // SDL_PHY_SDU
  
  RECEIVE , // SnetNodeCommand
  0x02    , // SDL_PHY_PDU
  0xC0    , // SDL_PHY_SDU
};
#define PHY_TEST_IN_LEN (sizeof(phyTestInData) / sizeof(phyTestInData[0]))

static void setupPhyTestIn(void)
{
  FILE *phyTestIn = NULL;
  uint8_t i;

  assert((phyTestIn = fopen(PHY_TEST_IN_NAME, "w")));
  for (i = 0; i < PHY_TEST_IN_LEN; i ++) {
    fputc(phyTestInData[i], phyTestIn);
  }
  fflush(phyTestIn);
}

static void tearDownPhyTestIn(void)
{
  unlink(PHY_TEST_IN_NAME);
}

static void failureHandler(void)
{
  tearDownPhyTestIn();
}

// -----------------------------------------------------------------------------
// Tests

int sanityTest(void)
{
  // We should be able send an error checking signal to ourselves.
  expectEquals(kill(getpid(), 0), 0);

  // Since we sign ourselves up for the CHILD_ALERT_SIGNAL, we should
  // handle the signal by reading from the PARENT_TO_CHILD_FD, i.e., stdin
  // in this test.
  expectEquals(snetChildAlert(getpid()), 0);

  return 0;
}

int receiveTest(void)
{
  // We should be able to send an error checking signal to ourselves.
  expectEquals(kill(getpid(), 0), 0);

  // We should be able to receive something.
  // See phyTestTxtData for these values.
  expectEquals(snetChildAlert(getpid()), 0);
  expectEquals(phyReceiveDataLength, 1);
  expectEquals(phyReceiveData[0], 0x80);

  return 0;
}

int transmitTest(void)
{
  // We should be able to send an error checking signal to ourselves.
  expectEquals(kill(getpid(), 0), 0);

  // We should be able to send something and have it written to stderr,
  // per the correct fd passed to this executable via the makefile.
  expectEquals(snetChildAlert(getpid()), 0);

  // Per the snet-internal.h definition, upon successfully transmitting,
  // we send outselves another CHILD_ALERT_SIGNAL so that we can make
  // sure that we actually sent something. Otherwise, we would most likely
  // be sending the CHILD_ALERT_SIGNAL to make or the terminal. This would
  // probably lead to incorrectly failing tests.
  // See the phyTestTxtData array for these values.
  expectEquals(phyReceiveDataLength, 1);
  expectEquals(phyReceiveData[0], 0xC0);

  return 0;
}

int main(void)
{
  announce();

  setFailureHandler(failureHandler);
  setupPhyTestIn();

  run(sanityTest);
  run(receiveTest);
  run(transmitTest);

  tearDownPhyTestIn();

  return 0;
}

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

#include "snet.h"
#include "snet-internal.h" // snetChildAlert
#include "sdl-types.h"

// Keep this inline with PHY_TEST_TXT in the makefile.
// This test reads from this file for the PARENT_TO_CHILD data.
#define PHY_TEST_TXT_NAME "phy-test.txt"

// -----------------------------------------------------------------------------
// Stubs

int sdlLogDump(void) { return 0; }
int sdlLogTx(unsigned char *bytes, int length) { return 0; }
int sdlLogRx(unsigned char *bytes, int length) { return 0; }

void sdlPhyReceiveIsr(uint8_t *data, uint8_t count)
{
}

// -----------------------------------------------------------------------------
// Utility

static const uint8_t phyTestTxtData[] = {
  NOOP, // First command is a NOOP command, i.e., do nothing.
};
#define PHY_TEST_TXT_LEN (sizeof(phyTestTxtData) / sizeof(phyTestTxtData[0]))

static void setupPhyTestTxt(void)
{
  FILE *phyTestTxt = NULL;
  uint8_t i;

  assert((phyTestTxt = fopen(PHY_TEST_TXT_NAME, "w")));
  for (i = 0; i < PHY_TEST_TXT_LEN; i ++) {
    fputc(phyTestTxtData[i], phyTestTxt);
  }
  fflush(phyTestTxt);
}

static void tearDownPhyTestTxt(void)
{
  unlink(PHY_TEST_TXT_NAME);
}

static void failureHandler(void)
{
  tearDownPhyTestTxt();
}

// -----------------------------------------------------------------------------
// Tests

int transmitTest(void)
{
  // We should be able send an error checking signal to ourselves.
  expectEquals(kill(getpid(), 0), 0);

  // Since we sign ourselves up for the CHILD_ALERT_SIGNAL, we should
  // handle the signal by reading from the PARENT_TO_CHILD_FD, i.e., stdin
  // in this test.
  expectEquals(snetChildAlert(getpid()), 0);

  return 0;
}

int main(void)
{
  announce();

  setFailureHandler(failureHandler);
  setupPhyTestTxt();

  run(transmitTest);

  tearDownPhyTestTxt();

  return 0;
}

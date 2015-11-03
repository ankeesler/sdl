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

#define SNET_CHILD_MAIN
#include "snet/src/child/child-main.h"  // snetChildSignalHandler
#include "snet/src/common/child-data.h" // SnetErrno_t
#include "snet/src/common/snet-command.h" // SDL_PHY_CHILD_COMMAND_NETIF_RECEIVE

#include "sdl-protocol.h" // SDL_PHY_SDU_MAX
#include "sdl-types.h" // SDL_SUCCESS
#include "phy.h" // sdlPhyTransmit

#include "cap/sdl-log.h" // sdlLogOn, sdlLogDump
#include "src/phy/phy-internal.h" // phyInit()

#include <signal.h> // sig_atmoic_t

// There are defined in the phy.c code.
extern volatile sig_atomic_t _childCommand, _childPayloadLength, _childPayload[];

// -----------------------------------------------------------------------------
// Stubs

int snetParentToChildFd = 3, snetChildToParentFd = 4;

SnetChildLog_t *snetChildLog = NULL;

void snetChildLogPrintf(SnetChildLog_t *log,
                        const char *format,
                        ...)
{
}

#define expectPhyReceiveIsr(data, count)              \
  do {                                                \
    expectEquals(phyReceiveDataLength, (count));      \
    expect(!memcmp(phyReceiveData, (data), (count))); \
  } while (0);
static uint8_t phyReceiveData[SDL_PHY_SDU_MAX];
static uint8_t phyReceiveDataLength = 0;
void sdlPhyReceiveIsr(uint8_t *data, uint8_t count)
{
  memcpy(phyReceiveData, data, count);
  phyReceiveDataLength = count;
}

static uint8_t childCommand, childPayloadLength, childPayload[255];
static SnetErrno_t childDataReceiveSnetErrno = 0;
SnetErrno_t snetChildDataReceive(int fd,
                                 uint8_t *command,
                                 uint8_t *payloadLength,
                                 uint8_t *payload)
{
  expectEquals(fd, snetParentToChildFd);

  *command = childCommand;
  *payloadLength = childPayloadLength;
  memcpy(payload, childPayload, *payloadLength);

  return childDataReceiveSnetErrno;
}

static SnetErrno_t childDataSendSnetErrno = 0;
static uint8_t childSendCommand, childSendPayloadLength, childSendPayload[255];
SnetErrno_t snetChildDataSend(int fd,
                              pid_t childPid,
                              uint8_t command,
                              uint8_t payloadLength,
                              uint8_t *payload)
{
  expectEquals(fd, snetChildToParentFd);

  expectEquals(command, childSendCommand);
  expectEquals(payloadLength, childSendPayloadLength);
  expect(!memcmp(payload, childSendPayload, childSendPayloadLength));

  return childDataSendSnetErrno;
}

// -----------------------------------------------------------------------------
// Utility

#define callChildSignalHandler(command, payloadLength, payload) \
  do {                                                          \
    childCommand       = (command);                             \
    childPayloadLength = (payloadLength);                       \
    memcpy(childPayload, (payload), (payloadLength));           \
    snetChildSignalHandler(SNET_CHILD_SIGNAL_ALERT);            \
  } while(0);

// -----------------------------------------------------------------------------
// Tests

int receiveTest(void)
{
  uint8_t shortPayload = 0xAC;
  uint8_t longPayload[] = { 5, 4, 3, 2, 1, };

  expectEquals(phyInit(), SDL_SUCCESS);

  // We should receive a signal from our parent.
  callChildSignalHandler(SNET_CHILD_COMMAND_NETIF_RECEIVE,
                         1,
                         &shortPayload);
  expectEquals(_childCommand, SNET_CHILD_COMMAND_NETIF_RECEIVE);
  expectEquals(_childPayloadLength, 1);
  expectEquals(_childPayload[0] & 0xFF, shortPayload);

  // We should call the pseudo-isr.
  expectPhyReceiveIsr(&shortPayload, 1);

  // Again again!
  callChildSignalHandler(SNET_CHILD_COMMAND_NETIF_RECEIVE,
                         5,
                         &longPayload);
  expectEquals(_childCommand, SNET_CHILD_COMMAND_NETIF_RECEIVE);
  expectEquals(_childPayloadLength, 5);
  expect(!memcmp((uint8_t *)_childPayload, longPayload, 5));
  expectPhyReceiveIsr(&longPayload, 5);

  // One last time.
  callChildSignalHandler(SNET_CHILD_COMMAND_NETIF_RECEIVE,
                         1,
                         &shortPayload);
  expectEquals(_childCommand, SNET_CHILD_COMMAND_NETIF_RECEIVE);
  expectEquals(_childPayloadLength, 1);
  expectEquals(_childPayload[0] & 0xFF, shortPayload);
  expectPhyReceiveIsr(&shortPayload, 1);

  return 0;
}

int transmitTest(void)
{
  uint8_t shortPayload = 0xAC;

  childSendCommand = 0;
  childSendPayloadLength = 0;
  childSendPayload[0] = 0;
  expectEquals(sdlPhyTransmit(&shortPayload, 1), SDL_SUCCESS);
  
  return 0;
}

int logTest(void)
{
  expect(sdlLogOn());
  sdlLogDump();

  // Sanity check.
  expect(!system("grep -q CAPTURE " SDL_LOG_FILE));
  expect( system("grep -q TUNA    " SDL_LOG_FILE));
  
  // File.
  expect(!system("grep -q -E '\\([0-9]+.[0-9]+\\) RX \\[0xAC\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) RX \\[0x05, 0x04, 0x03, 0x02, 0x01\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) RX \\[0xAC\\]' " SDL_LOG_FILE));

  return 0;
}

int main(void)
{
  announce();

  run(receiveTest);
  run(transmitTest);

  run(logTest);

  return 0;
}

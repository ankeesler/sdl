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

#include "inc/sdl-protocol.h" // SDL_PHY_SDU_MAX, SDL_PHY_PDU_LEN
#include "inc/sdl-types.h" // SDL_SUCCESS
#include "inc/phy.h" // sdlPhyTransmit

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

void snetChildLogPrintBytes(SnetChildLog_t *log,
                            uint8_t *bytes,
                            uint8_t count)
{
}

#define expectPhyReceiveIsr(data, count)              \
  do {                                                \
    expectEquals(phyReceiveDataLength, (count));      \
    expect(!memcmp(phyReceiveData, (data), (count))); \
  } while (0);
static uint8_t phyReceiveData[SDL_PHY_SDU_MAX];
static uint8_t phyReceiveDataLength = 0;
void phyReceiveIsr(uint8_t *data, uint8_t count)
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
static uint8_t expectedChildSendCommand;
static uint8_t expectedChildSendPayloadLength;
static uint8_t expectedChildSendPayload[255];
SnetErrno_t snetChildDataSend(int fd,
                              pid_t childPid,
                              uint8_t actualChildSendCommand,
                              uint8_t actualChildSendPayloadLength,
                              uint8_t *actualChildSendPayload)
{
  expectEquals(fd, snetChildToParentFd);

  expectEquals(actualChildSendCommand, expectedChildSendCommand);
  expectEquals(actualChildSendPayloadLength, expectedChildSendPayloadLength);

  // phy pdu length byte + sdu
  expect(!memcmp(actualChildSendPayload,
                 expectedChildSendPayload,
                 expectedChildSendPayloadLength));

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
  uint8_t shortPacket[] = { 0x02, 0xAC, };
  uint8_t longPacket[] = { 6, 5, 4, 3, 2, 1, };

  expectEquals(phyInit(), SDL_SUCCESS);

  // We should receive a signal from our parent.
  callChildSignalHandler(SNET_CHILD_COMMAND_NETIF_RECEIVE,
                         sizeof(shortPacket),
                         &shortPacket);
  expectEquals(_childCommand, SNET_CHILD_COMMAND_NETIF_RECEIVE);
  expectEquals(_childPayloadLength, sizeof(shortPacket));
  expect(!memcmp((uint8_t *)_childPayload, shortPacket, sizeof(shortPacket)));

  // We should call the pseudo-isr.
  expectPhyReceiveIsr(&shortPacket[SDL_PHY_PDU_LEN],
                      sizeof(shortPacket) - SDL_PHY_PDU_LEN);

  // Again again!
  callChildSignalHandler(SNET_CHILD_COMMAND_NETIF_RECEIVE,
                         sizeof(longPacket),
                         &longPacket);
  expectEquals(_childCommand, SNET_CHILD_COMMAND_NETIF_RECEIVE);
  expectEquals(_childPayloadLength, sizeof(longPacket));
  expect(!memcmp((uint8_t *)_childPayload, longPacket, sizeof(longPacket)));
  expectPhyReceiveIsr(&longPacket[SDL_PHY_PDU_LEN],
                      sizeof(longPacket) - SDL_PHY_PDU_LEN);

  // One last time.
  callChildSignalHandler(SNET_CHILD_COMMAND_NETIF_RECEIVE,
                         sizeof(shortPacket),
                         &shortPacket);
  expectEquals(_childCommand, SNET_CHILD_COMMAND_NETIF_RECEIVE);
  expectEquals(_childPayloadLength, sizeof(shortPacket));
  expect(!memcmp((uint8_t *)_childPayload, shortPacket, sizeof(shortPacket)));
  expectPhyReceiveIsr(&shortPacket[SDL_PHY_PDU_LEN],
                      sizeof(shortPacket) - SDL_PHY_PDU_LEN);

  return 0;
}

int transmitTest(void)
{
  uint8_t shortPayload = 0xAC;
  uint8_t longPayload[] = {5, 4, 3, 2, 1,};

  expectedChildSendCommand = SNET_CHILD_COMMAND_NETIF_TRANSMIT;
  expectedChildSendPayloadLength = sizeof(shortPayload) + SDL_PHY_PDU_LEN;
  expectedChildSendPayload[0] = expectedChildSendPayloadLength;
  expectedChildSendPayload[SDL_PHY_PDU_LEN] = shortPayload;
  expectEquals(sdlPhyTransmit(&shortPayload, sizeof(shortPayload)), SDL_SUCCESS);
  
  expectedChildSendCommand = SNET_CHILD_COMMAND_NETIF_TRANSMIT;
  expectedChildSendPayloadLength = sizeof(longPayload) + SDL_PHY_PDU_LEN;
  expectedChildSendPayload[0] = expectedChildSendPayloadLength;
  memcpy(&expectedChildSendPayload[SDL_PHY_PDU_LEN],
         longPayload,
         sizeof(longPayload));
  expectEquals(sdlPhyTransmit(longPayload, sizeof(longPayload)), SDL_SUCCESS);

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
  expect(!system("grep -q -E '\\([0-9]+.[0-9]+\\) RX \\[0x02, 0xAC\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) RX \\[0x06, 0x05, 0x04, 0x03, 0x02, 0x01\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) RX \\[0x02, 0xAC\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) TX \\[0x02, 0xAC\\]' " SDL_LOG_FILE));
  expect(!system("grep -q -E '\\([0-9]+\\.[0-9]+\\) TX \\[0x06, 0x05, 0x04, 0x03, 0x02, 0x01\\]' " SDL_LOG_FILE));

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

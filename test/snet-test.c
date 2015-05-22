//
// snet-test.c
//
// Andrew Keesler
//
// February 8, 2015
//
// SNET test.
//

#include <unit-test.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

#define __SNET_C__
#include "snet/snet.h"
#include "nodes/server.h"
#include "mac.h"

#define RUNNING(node) (kill(node->pid, 0) == 0)
#define STOP(server) kill(server->pid, SERVER_OFF_SIGNAL)

SnetNode *server = NULL;
SnetNode *server1 = NULL, *server2 = NULL;

// SNET test harness.
extern void printSignalData(void);

// Stubs.
SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length) { return SDL_SUCCESS; }

static void failureHandler(void)
{
  if (server)  kill(server->pid, SIGTERM);
  if (server1) kill(server1->pid, SIGTERM);
  if (server2) kill(server2->pid, SIGTERM);

  while (wait(NULL) != -1) ;
}

int singleNodeTest(void)
{
  server = NULL;
  snetManagementInit();

  // Make the node.
  server = snetNodeMake("build/server/server", "server");
  expect((int)server);
  expect(snetManagementSize() == 0);

  // We can't remove the node from the network since it isn't on it.
  expect(snetNodeStop(server));
  expect(snetManagementSize() == 0);

  // But we should be able to add it.
  expect(!snetNodeStart(server));
  expect(snetManagementSize() == 1);
  
  // The node should be running.
  expect(RUNNING(server));
  
  // We shouldn't be able to re-add the node to the network yet.
  expect(snetNodeStart(server));
  expect(snetManagementSize() == 1);
  
  // The server should still be running.
  expect(RUNNING(server));
  
  // But we should be able to remove it.
  expect(!snetNodeStop(server));
  expect(snetManagementSize() == 0);
  
  // The server should not be running.
  expect(!RUNNING(server));

  // And we should be able to add it again.
  expect(!snetNodeStart(server));
  expect(snetManagementSize() == 1);
  
  // The server should be running.
  expect(RUNNING(server));
  
  // And with the server, we should be able to stop it.
  expect(!STOP(server));

  // Then, the server should stop running after some time.
  usleep(SERVER_DUTY_CYCLE_US);
  expect(!RUNNING(server));

  // There should not have been any nodes left on the network.
  expect(snetManagementDeinit() == 0);

  return 0;
}

int doubleNodeTest(void)
{
  server1 = server2 = NULL;
  snetManagementInit();  

  // Create two servers.
  expect((int)(server1 = snetNodeMake("build/server/server", "server1")));
  expect((int)(server2 = snetNodeMake("build/server/server", "server2")));
  expect(snetManagementSize() == 0);
  
  // Add the servers to the network. They will spin.
  expect(!snetNodeStart(server1));
  expect(!snetNodeStart(server2));
  expect(snetManagementSize() == 2);
  
  // Both of the servers should be running.
  expect(RUNNING(server1));
  expect(RUNNING(server2));

  // Remove them both, in opposite order, from the network.
  expect(!snetNodeStop(server2));
  expect(!snetNodeStop(server1));
  expect(snetManagementSize() == 0);
  
  // Both of the servers should not be running.
  expect(!RUNNING(server1));
  expect(!RUNNING(server2));
  
  // Add both of the servers again. They should both start running.
  expect(!snetNodeStart(server1));
  expect(!snetNodeStart(server2));
  expect(snetManagementSize() == 2);
  expect(RUNNING(server1));
  expect(RUNNING(server2));

  // Turn off the servers.
  expect(!STOP(server1));
  expect(!STOP(server2));

  // Both of the servers should stop running, and the number
  // of nodes in the network should drop to 0.
  while (RUNNING(server1)) ;
  while (RUNNING(server2)) ;
  expect(!snetManagementSize());

  expect(snetManagementDeinit() == 0);

  return 0;
}

int badNodeTest(void)
{
  snetManagementInit();

  // We should not be able to add or remove a NULL pointer.
  expect(snetNodeStart(NULL));
  expect(snetNodeStop(NULL));

  // We should not be able to add or remove a pointer that is
  // not in the nodePool.
  int i = 1;
  expect(snetNodeStart((SnetNode*)&i));
  expect(snetNodeStop((SnetNode*)&i));

  expect(snetManagementDeinit() == 0);
  expect(!snetManagementSize());

  return 0;
}

int noopTest(void)
{
  server = NULL;
  snetManagementInit();

  // Create a server.
  expect((int)(server = snetNodeMake("build/server/server", "server")));
  expect(snetManagementSize() == 0);

  // Can't send a command if a node is not on a network.
  expect(snetNodeCommand(server, NOOP));
  
  // Add the node to the network.
  expect(!snetNodeStart(server));

  // Check that they are running.
  expect(snetManagementSize() == 1);
  expect(RUNNING(server));
  
  // Send a NOOP.
  expect(!snetNodeCommand(server, NOOP));

  // Since the command does nothing, the nodes should both still be running.
  expect(snetManagementSize() == 1);
  expect(RUNNING(server));
  
  // Done.
  expect(snetManagementDeinit() == 1);
  expect(!snetManagementSize());

  return 0;
}

int receiveTest(void)
{
  SdlPacket packet;
  uint8_t serverCommand[SDL_PHY_SDU_MAX + 1];

  // The basic SDL header used for this test.
  packet.type = SDL_PACKET_TYPE_DATA;
  packet.sequence = 0xABCD;
  packet.source = 0x01234567;
  packet.destination = 0xFFFFFFFF;
  packet.dataLength = 1;

  server = NULL;
  snetManagementInit();

  // Create a server.
  expect((int)(server = snetNodeMake("build/server/server", "server")));
  expect(snetManagementSize() == 0);

  // Boot the node.
  expect(!snetNodeStart(server));
  expect(snetManagementSize() == 1);
  expect(RUNNING(server));

  // TODO: why do we need this?
  usleep(SERVER_DUTY_CYCLE_US);

  // Build the SDL packet in a flat buffer.
  sdlPacketToFlatBuffer(&packet, serverCommand + 1);
  serverCommand[0] = SDL_PHY_PDU_LEN + SDL_MAC_PDU_LEN + 1; // packet length

  // Send a noop command to the server to make sure it continues to run.
  serverCommand[SDL_PHY_PDU_LEN + SDL_MAC_PDU_LEN] = SERVER_NOOP_COMMAND;
  expect(!snetNodeCommand(server, RECEIVE, serverCommand));
  usleep(SERVER_DUTY_CYCLE_US);
  expect(RUNNING(server));

  // Send another noop command since I am skeptical.
  serverCommand[SDL_PHY_PDU_LEN + SDL_MAC_PDU_LEN] = SERVER_NOOP_COMMAND;
  expect(!snetNodeCommand(server, RECEIVE, serverCommand));
  usleep(SERVER_DUTY_CYCLE_US);
  expect(RUNNING(server));

  // Send an off command to the server and it should turn off within the
  // duty cycle window.
  serverCommand[SDL_PHY_PDU_LEN + SDL_MAC_PDU_LEN] = SERVER_OFF_COMMAND;
  expect(!snetNodeCommand(server, RECEIVE, serverCommand));
  usleep(SERVER_DUTY_CYCLE_US);
  expect(!RUNNING(server));

  // Done.
  expect(snetManagementDeinit() == 0);
  expect(!snetManagementSize());

  return 0;
}

int transmitTest(void)
{
  SdlPacket packet;
  uint8_t serverCommand[SDL_PHY_SDU_MAX + 1];

  // Bring up two servers.
  expect((int)(server1 = snetNodeMake("server1", "build/server/server")));
  expect((int)(server2 = snetNodeMake("server2", "build/server/server")));

  // Boot the servers.
  expect(!snetNodeStart(server1));
  expect(RUNNING(server1));
  expect(!snetNodeStart(server2));
  expect(RUNNING(server2));

  // The basic SDL header used for this test.
  packet.type = SDL_PACKET_TYPE_DATA;
  packet.sequence = 0xABCD;
  packet.source = 0x01234567;
  packet.destination = 0xFFFFFFFF;
  packet.dataLength = 1;
  packet.data[0] = SERVER_OFF_COMMAND;
  sdlPacketToFlatBuffer(&packet, serverCommand + 1);
  serverCommand[0] = SDL_PHY_PDU_LEN + SDL_MAC_PDU_LEN + 1; // packet length

  // Transmit the off command from server1 to server2.
  expect(!snetNodeCommand(server1, TRANSMIT, serverCommand));

  // After a duty cycle, server2 should be off.
  usleep(SERVER_DUTY_CYCLE_US);
  expect(RUNNING(server1));
  expect(!RUNNING(server2));

  // Tear down the network.
  expect(snetManagementDeinit() == 1);
  expect(!snetManagementSize());

  return 0;
}

int main(void)
{
  announce();

  setFailureHandler(failureHandler);

  run(singleNodeTest);
  run(doubleNodeTest);
  run(badNodeTest);
  
  run(noopTest);
  
  run(receiveTest);
  run(transmitTest);

  return 0;
}

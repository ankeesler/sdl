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

#define __SNET_TEST_C__
#include "snet/snet.h"
#include "nodes/server.h"

#define RUNNING(node) (kill(node->pid, 0) == 0)
#define STOP(server) kill(server->pid, SIGUSR2)

SnetNode *server = NULL, *client = NULL;
SnetNode *server1 = NULL, *server2 = NULL;

static void failureHandler(void)
{
  if (server)
    kill(server->pid, SIGTERM);
  if (client)
    kill(client->pid, SIGTERM);
  if (server1)
    kill(server1->pid, SIGTERM);
  if (server2)
    kill(server2->pid, SIGTERM);

  while (wait(NULL) != -1) ;
}

int singleNodeTest(void)
{
  server = client = NULL;
  snetManagementInit();

  // Make the node.
  server = snetNodeMake("build/server/server", "server");
  expect((int)server);
  expect(snetNodeCount() == 0);

  // We can't remove the node from the network since it isn't on it.
  expect(snetNodeRemove(server));
  expect(snetNodeCount() == 0);

  // But we should be able to add it.
  expect(!snetNodeAdd(server));
  expect(snetNodeCount() == 1);
  
  // The node should be running.
  expect(RUNNING(server));
  
  // We shouldn't be able to re-add the node to the network yet.
  expect(snetNodeAdd(server));
  expect(snetNodeCount() == 1);
  
  // The server should still be running.
  expect(RUNNING(server));
  
  // But we should be able to remove it.
  expect(!snetNodeRemove(server));
  expect(snetNodeCount() == 0);
  
  // The server should not be running.
  expect(!RUNNING(server));

  // And we should be able to add it again.
  expect(!snetNodeAdd(server));
  expect(snetNodeCount() == 1);
  
  // The server should be running.
  expect(RUNNING(server));
  
  // And with the server, we should be able to stop it.
  expect(!STOP(server));

  // Then, the server should stop running after some time.
  usleep(SERVER_DUTY_CYCLE_US);
  expect(!RUNNING(server));

  snetManagementDeinit();

  return 0;
}

int doubleNodeTest(void)
{
  server1 = server2 = NULL;
  snetManagementInit();  

  // Create a client and a server.
  expect((int)(server1 = snetNodeMake("build/server/server", "server1")));
  expect((int)(server2 = snetNodeMake("build/server/server", "server2")));
  expect(snetNodeCount() == 0);
  
  // Add the servers to the network. They will spin.
  expect(!snetNodeAdd(server1));
  expect(!snetNodeAdd(server2));
  expect(snetNodeCount() == 2);
  
  // Both of the servers should be running.
  expect(RUNNING(server1));
  expect(RUNNING(server2));

  // Remove them both, in opposite order, from the network.
  expect(!snetNodeRemove(server2));
  expect(!snetNodeRemove(server1));
  expect(snetNodeCount() == 0);
  
  // Both of the servers should not be running.
  expect(!RUNNING(server1));
  expect(!RUNNING(server2));
  
  // Add both of the servers again. They should both start running.
  expect(!snetNodeAdd(server1));
  expect(!snetNodeAdd(server2));
  expect(snetNodeCount() == 2);
  expect(RUNNING(server1));
  expect(RUNNING(server2));

  // Turn off the servers.
  STOP(server1);
  STOP(server2);

  // Both of the servers should stop running, and the number
  // of nodes in the network should drop to 0.
  while (RUNNING(server1)) ;
  while (RUNNING(server2)) ;
  while (snetNodeCount()) ;

  snetManagementDeinit();

  return 0;
}

int badNodeTest(void)
{
  snetManagementInit();

  // We should not be able to add or remove a NULL pointer.
  expect(snetNodeAdd(NULL));
  expect(snetNodeRemove(NULL));

  // We should not be able to add or remove a pointer that is
  // not in the nodePool.
  int i = 1;
  expect(snetNodeAdd((SnetNode*)&i));
  expect(snetNodeRemove((SnetNode*)&i));

  snetManagementDeinit();

  return 0;
}

int noopTest(void)
{
  client = server = NULL;
  snetManagementInit();

  // Create a client and a server.
  expect((int)(client = snetNodeMake("build/client/client", "client")));
  expect((int)(server = snetNodeMake("build/server/server", "server")));
  expect(snetNodeCount() == 0);

  // Can't send a command if a node is not on a network.
  expect(snetNodeCommand(server, NOOP));
  expect(snetNodeCommand(client, NOOP));
  
  // Add the nodes to the network.
  expect(!snetNodeAdd(server));
  expect(!snetNodeAdd(client));
  
  // Send a NOOP.
  expect(!snetNodeCommand(server, NOOP));
  expect(!snetNodeCommand(client, NOOP));
  
  snetManagementDeinit();

  return 0;
}

int receiveTest(void)
{
  server = NULL;
  snetManagementInit();

  // Create a client and a server.
  expect((int)(server = snetNodeMake("build/server/server", "server")));
  expect(snetNodeCount() == 0);

  // Add the nodes to the network.
  expect(!snetNodeAdd(server));
  expect(snetNodeCount() == 1);
  expect(RUNNING(server));
  
  // Tell the server to receive something
  unsigned char macHeader[] = {0x00, 0x00,             // frame control
                               0x00, 0x01,             // sequence number
                               0x00, 0x00, 0x00, 0x01, // source address
                               0x00, 0x00, 0x00, 0x02, // destination address
                             };
  expect(!snetNodeCommand(server, RECEIVE, 8, macHeader));
  
  // Tell the server to stop and wait for it.
  STOP(server);
  while (RUNNING(server)) ;
  while (snetNodeCount()) ;
  
  snetManagementDeinit();
  
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

  return 0;
}

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

#define __SNET_TEST_C__
#include "snet/snet.h"

int singleNodeTest(void)
{
  SnetNode *node;

  snetManagementInit();

  // Make the node.
  node = snetNodeMake("build/server/server", "server");
  expect((int)node);
  expect(snetNodeCount() == 0);

  // We can't remove the node from the network since it isn't on it.
  expect(snetNodeRemove(node));
  expect(snetNodeCount() == 0);

  // But we should be able to add it.
  expect(!snetNodeAdd(node));
  expect(snetNodeCount() == 1);

  // We shouldn't be able to re-add the node to the network yet.
  expect(snetNodeAdd(node));
  expect(snetNodeCount() == 1);

  // But we should be able to remove it.
  expect(!snetNodeRemove(node));
  expect(snetNodeCount() == 0);

  // And we should be able to add it again.
  expect(!snetNodeAdd(node));
  expect(snetNodeCount() == 1);

  snetManagementDeinit();

  return 0;
}

int doubleNodeTest(void)
{
  SnetNode *client, *server;

  snetManagementInit();  

  // Create a client and a server.
  expect((int)(client = snetNodeMake("build/client/client", "client")));
  expect((int)(server = snetNodeMake("build/server/server", "server")));
  expect(snetNodeCount() == 0);
  
  // Add them both to the network.
  expect(!snetNodeAdd(client));
  expect(!snetNodeAdd(server));
  expect(snetNodeCount() == 2);

  // Remove them both, in opposite order, from the network.
  expect(!snetNodeRemove(server));
  expect(!snetNodeRemove(client));
  expect(snetNodeCount() == 0);

  // Add the client, then add the server, the remove the client.
  expect(!snetNodeAdd(client));
  expect(!snetNodeAdd(server));
  expect(!snetNodeRemove(client));
  expect(snetNodeCount() == 1);
  
  // The number of nodes should eventually drop to 0, since the server
  // should return.
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
  SnetNode *client, *server;

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

int oneWayTest(void)
{
  SnetNode *client, *server;

  snetManagementInit();

  // Create a client and a server.
  expect((int)(client = snetNodeMake("build/client/client", "client")));
  expect((int)(server = snetNodeMake("build/server/server", "server")));
  expect(snetNodeCount() == 0);

  // Add the nodes to the network.
  expect(!snetNodeAdd(server));
  expect(!snetNodeAdd(client));
  
  // Tell the client to send the server something.
  unsigned char macHeader[] = {0xFF, 0xFF, 0xFF, 0xFF,  // destination address
                               0x00, 0x00, 0x00, 0x00}; // sequence
  expect(!snetNodeCommand(server, TRANSMIT, 8, macHeader));
  
  snetManagementDeinit();
  
  return 0;
}

int main(void)
{
  announce();

  run(singleNodeTest);
  run(doubleNodeTest);

  run(badNodeTest);

  run(noopTest);
  
  run(oneWayTest);

  return 0;
}

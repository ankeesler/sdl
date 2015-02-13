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

int main(void)
{
  announce();
  run(singleNodeTest);
  return 0;
}

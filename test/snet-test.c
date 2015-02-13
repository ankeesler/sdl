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

  snetNodeAdd(node);
  expect(snetNodeCount() == 1);

  snetNodeRemove(node);
  expect(snetNodeCount() == 0);

  snetManagementDeinit();

  return 0;
}

int main(void)
{
  announce();
  run(singleNodeTest);
  return 0;
}

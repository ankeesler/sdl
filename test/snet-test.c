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
  SnetNode node = {
    "build/server/server",  // image
    "server"                // name
  };

  expect(snetNodeCount() == 0);
  snetNodeAdd(&node);
  expect(snetNodeCount() == 1);
  snetNodeRemove(&node);
  expect(snetNodeCount() == 0);

  return 0;
}

int main(void)
{
  announce();
  run(singleNodeTest);
  return 0;
}

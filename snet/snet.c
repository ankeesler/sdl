//
// snet.c
//
// Andrew Keesler
//
// February 8, 2015
//
// SNET implementations.
//

#include <stdio.h>

#include "snet.h"

static int nodeCount = 0;

void snetNodeAdd(SnetNode *node)
{
  nodeCount ++;
}

void snetNodeRemove(SnetNode *node)
{
  nodeCount --;
}

int snetNodeCount(void)
{
  return nodeCount;
}

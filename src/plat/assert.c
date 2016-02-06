//
// assert.c
//
// Andrew Keesler
//
// Wednesday, November 18, 2015
//
// Platform assert utilities.
//

#include <stdlib.h> // exit()

#include "src/snet/src/common/child-data.h" // SNET_CHILD_EXIT_STATUS_ASSERT_ERR
#include "src/snet/src/child/child-log.h"   // snetChildLogPrintf
#include "src/snet/src/child/child-main.h"  // snetChildLog

#include "assert.h"

void sdlAssert(bool expression, int line, const char *file)
{
  if (!expression) {
    snetChildLogPrintf(snetChildLog, "Assert failed at line %d in %s.\n",
                       line, file);
    exit(SNET_CHILD_EXIT_STATUS_ASSERT_ERR);
  }
}

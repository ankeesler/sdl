//
// snet-expect.h
//
// Andrew Keesler
//
// July 18, 2015
//
// SNET expect framework.
//

#ifndef __SNET_EXPECT_H__
#define __SNET_EXPECT_H__

#include "snet.h"

#include <stdio.h>

#define DEFAULT_TIMEOUT_US (3000000) // 3 seconds

// Initialize the SNET Expect framework.
// If someone else is using the SnetNodeUartIsr, the initialization will fail.
// This function also initializes the SNET management layer.
// Also logs the UART data to a file with the argument name.
SdlStatus snetExpectInit(FILE *file);

bool snetExpect(SnetNode *node, const char *regex, uint32_t timeoutUS);

#endif /* __SNET_EXPECT_H__ */

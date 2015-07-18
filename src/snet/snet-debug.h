//
// snet-debug.h
//
// AC Keesler
//
// May 23, 2015
//
// Debugging utilities for SNET applications.
//

#ifndef __SNET_DEBUG_H__
#define __SNET_DEBUG_H__

#include <stdint.h>
#include <stdio.h>

extern const char *signalNames[];

#ifdef SNET_TEST

// Print some SDL PHY data.
// This means the first byte is the length of the total packet.
void printSdlData(FILE *file, uint8_t *bytes);

#endif


#endif /* __SNET_DEBUG_H__ */

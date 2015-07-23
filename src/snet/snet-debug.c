//
// snet-debug.c
//
// AC Keesler
//
// May 23, 2015
//
// Debugging utilities for SNET applications.
//

#include "snet-debug.h"

void printSdlData(FILE *file, uint8_t *bytes)
{
  uint8_t i = 0;
  while (i < bytes[0]) fprintf(file, " 0x%02X", bytes[i++]);
}

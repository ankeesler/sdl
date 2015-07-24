//
// sensor-sink-common.c
//
// Andrew Keesler
//
// July 24, 2015
//
// Common routines to the Sensor/Sink scenario.
//

#include "sdl.h"

#include <string.h> // strlen()
#include <stdarg.h> // va_list, va_start(), va_arg(), va_end()
#include <stdio.h>  // vsnprint()

void sensorSinkEncryptOrDecrypt(uint8_t *data, uint8_t length, const char *key)
{
  uint8_t dataI, keyI, keyLength;

  keyLength = strlen(key);

  // Only the highest of encryption standards.
  for (dataI = 0; dataI < length; dataI ++) {
    data[dataI] ^= key[keyI];

    if (++keyI == keyLength) {
      keyI = 0;
    }
  }
}

void sensorSinkPrintf(const char *format, ...)
{
  char data[UINT8_MAX];
  va_list args;

  va_start(args, format);

  vsnprintf(data, sizeof(data), format, args);

  sdlUartTransmit((uint8_t *)data, strlen(data));

  va_end(args);
}


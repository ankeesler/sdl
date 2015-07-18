//
// uart.h
//
// Andrew Keesler
//
// July 13, 2015
//
// UART driver for simulated node.
//

#include <stdint.h>

// Transmit some bytes across the UART.
SdlStatus sdlUartTransmit(uint8_t *data, uint8_t length);

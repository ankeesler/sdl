//
// nvic.h
//
// Andrew Keesler
//
// November 16, 2015
// Flight back from Charlotte to Boston.
//
// Pseudo-ISR controller. Link from simulated network to stack.
//

#include <stdint.h>  // uint8_t
#include <stdbool.h> // bool

// Something has been received on the network interface.
void nvicNetifReceiveIsr(uint8_t *data, uint8_t dataLength);

// An LED was touched.
void nvicLedTouchedIsr(uint8_t led, bool on);

// Something was received over the UART.
void nvicUartIsr(uint8_t *data, uint8_t dataLength);

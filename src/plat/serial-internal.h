//
// serial-internal.h
//
// Andrew Keesler
//
// Sunday, December 13, 2015
//
// Serial internal declarations.
//

#include <stdint.h> // uint32_t
#include <signal.h> // sig_atmoic_t

// Counters values for serial code.
volatile sig_atomic_t platSerialRxOverflow;
volatile sig_atomic_t platSerialRxDrop;
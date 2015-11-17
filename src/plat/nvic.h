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

// Something has been received on the network interface.
void nvicNetifReceiveIsr(uint8_t *data, uint8_t dataLength);

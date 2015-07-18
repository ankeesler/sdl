//
// phy-internal.h
//
// Andrew Keesler
//
// July 13, 2015
//
// Internal defines for PHY stuff.
//

// IPC file descriptors for communicating with our parent.
extern int parentToChildFd, childToParentFd;

// The physical layer has received length number of bytes.
// The data points to the first byte past the PHY PDU.
// The length is the length of the data vector.
void sdlPhyReceiveIsr(uint8_t *data, uint8_t length);

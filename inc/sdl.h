//
// sdl.h
//
// Andrew Keesler
//
// July 18, 2015
//
// Simulated Data Link API.
//

//
// A Simulated Data Link layer.
//   -         -    -
//
// The SDL protocol stack is meant to emulate a hardware network interface.
// Users should be able to swap out their data layer implementations for
// the SDL implementation and simulate their upper layer functionality
// on a POSIX-compliant platform.
//
// Most layer 3 or raw application code will interface directly with the
// SDL MAC layer, whose interface is defined in mac.h. For sample
// applications, see the test/app/ directory included in this repo.
//
// For layer 3 or raw application code wishing to interface more closely
// to the simulated layer 1 functionality, the SDL PHY layer interface
// is included here. This API provides access to the raw simulated
// networking interface that SDL provides.
//

// SDL Protocol definitions.
#include "sdl-protocol.h"

// SDL type definitions.
#include "sdl-types.h"

// SDL SNET definitions.
#include "src/snet/inc/snet-child.h"

// SDL PLAT layer interface.
#include "plat.h"

// SDL PHY layer interface.
#include "phy.h"

// SDL MAC layer interface.
#include "mac.h"

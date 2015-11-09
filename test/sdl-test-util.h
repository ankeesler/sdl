//
// sdl-test-util.h
//
// Andrew Keesler
//
// May 22, 2015
//
// SDL testing utilities.
//

#include <signal.h> // kill()

// Is a child node running?
#define RUNNING(node) (kill(node->pid, 0) == 0)

// SNET test harness.
extern void printSignalData(void);
extern void printNodeData(void);

// Apps.
#define SENSOR_IMAGE "build/sensor/sensor"

//
// sdl-test-util.h
//
// Andrew Keesler
//
// May 22, 2015
//
// SDL testing utilities.
//

// Is a child node running?
#define RUNNING(node) (kill(node->pid, 0) == 0)

// SNET test harness.
extern void printSignalData(void);
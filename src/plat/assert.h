//
// assert.h
//
// Andrew Keesler
//
// Wednesday, November 18, 2015
//
// Platform assert utilities.
//

#include <stdbool.h>

// If this fails, the child will exit safely with an error code.
#define sdlPlatAssert(e) sdlAssert(e, __LINE__, __FILE__)
void sdlAssert(bool expression, int line, const char *file);

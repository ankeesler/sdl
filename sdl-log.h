//
// sdl-log.h
//
// Andrew Keesler
//
// December 13, 2014
//
// Log the stuff that comes into the network.

/*
  SPECIFICATION

  When a packet it submitted to the simulated data link,
  this module records it.

  First, the logging should be started with sdlLogInit().
  Then, this module will log the time and the data injected into the network.
  The log can be dumped to a stream with sdlLogDump(). If a client wishes to
  save to a file, then they must pass a value for the macro SDL_LOG_FILE to
  the preprocessor.

  A client can turn on this feature with SDL_LOG.

  In order to log something, sdlLog*(bytes, length) should be called.

  This module is not thread safe.
*/

// Initialize the log.
// Returns 0 on success, 1 on failure.
int sdlLogInit(void);

// Returns whether or not network traffic is being logged.
int sdlLogOn(void);

// Dumps log to specified file. If no value is specified for
// macro SDL_LOG, then it will be dumped to stdout.
int sdlLogDump(void);

// Log transmits and receives.
// 0 for success, 1 for error.
int sdlLogTx(unsigned char *bytes, int length);
int sdlLogRx(unsigned char *bytes, int length);

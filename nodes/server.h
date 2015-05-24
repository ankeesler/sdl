//
// server.h
//
// Andrew Keesler
//
// Created April 24, 2015
//
// Server test application defines and test hooks.
//

#include <signal.h>

#define SERVER_DUTY_CYCLE_US (10000)

#define SERVER_OFF_SIGNAL (SIGUSR2)

#define SERVER_NOOP_COMMAND (0x00)
#define SERVER_OFF_COMMAND  (0x0A)

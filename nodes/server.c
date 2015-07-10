//
// server.c
//
// Andrew Keesler
//
// Created February 6. 2015
//
// Server test application.
//

#include <stdio.h>
#include <stdlib.h>

#include "mac.h"
#include "snet.h"
#include "server.h"

#include <signal.h> // sig_atomic_t

static volatile sig_atomic_t spin = 1;

int nodeState = 0;

#define NULL_BUTTON 0xFF
static uint8_t buttonPressed = NULL_BUTTON;

void sdlPhyButtonIsr(uint8_t button)
{
  button = buttonPressed;
}

void handler(int signal)
{
  if (signal == SERVER_OFF_SIGNAL) {
    spin = 0;
  }
}

static SdlStatus broadcastServerOff(void)
{
  uint8_t data[SDL_PHY_SDU_MAX];
  data[0] = SERVER_OFF_COMMAND;
  return sdlMacTransmit(SDL_PACKET_TYPE_DATA,
                        SDL_MAC_ADDRESS_BROADCAST,
                        data,
                        1);
}

int main(void)
{
  extern FILE *childLogFile;
  SdlPacket packet;

  sdlMacInit(getpid());

  signal(SERVER_OFF_SIGNAL, handler);

  fprintf(childLogFile, "server: up\n");
  while (spin) {
    // Sleep.
    usleep(SERVER_DUTY_CYCLE_US);

    // Maybe respond to a button press?
    if (buttonPressed == SERVER_OFF_BUTTON) {
      // Broadcast a SERVER_OFF_COMMAND.
      broadcastServerOff();
      buttonPressed = NULL_BUTTON;
    } else if (buttonPressed == SERVER_NOOP_BUTTON) {
      ; // noop
    }

    // Try to receive something.
    if (sdlMacReceive(&packet) == SDL_SUCCESS) {
      spin = (packet.data[0] != SERVER_OFF_COMMAND);
    }
  }

  exit(0);
}


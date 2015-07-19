//
// server.c
//
// Andrew Keesler
//
// Created February 6. 2015
//
// Server test application.
//

#include "sdl.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h> // sig_atomic_t
#include <string.h> // strlen
#include <unistd.h> // getpid

static volatile sig_atomic_t spin = 1;

int nodeState = 0;

#define NULL_BUTTON 0xFF
static uint8_t buttonPressed = NULL_BUTTON;

void sdlPhyButtonIsr(uint8_t button)
{
  buttonPressed = button;
}

void handler(int signal)
{
  if (signal == SERVER_OFF_SIGNAL) {
    spin = 0;
  }
}

static SdlStatus broadcastServerOff(void)
{
  SdlStatus status;
  uint8_t data[SDL_PHY_SDU_MAX];

  data[0] = SERVER_OFF_COMMAND;

  status =  sdlMacTransmit(SDL_PACKET_TYPE_DATA,
                           SDL_MAC_ADDRESS_BROADCAST,
                           data,
                           1);

  return status;
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
    } else if (buttonPressed == SERVER_UART_BUTTON1) {
      uint8_t *data = (uint8_t *)SERVER_UART_BUTTON1_STRING;
      sdlUartTransmit(data, strlen(SERVER_UART_BUTTON1_STRING));
      buttonPressed = NULL_BUTTON;
    } else if (buttonPressed == SERVER_UART_BUTTON2) {
      uint8_t *data = (uint8_t *)SERVER_UART_BUTTON2_STRING;
      sdlUartTransmit(data, strlen(SERVER_UART_BUTTON2_STRING));
      buttonPressed = NULL_BUTTON;
    }

    // Try to receive something.
    if (sdlMacReceive(&packet) == SDL_SUCCESS) {
      fprintf(childLogFile, "packet received: [0x%02X]", packet.data[0]);
      fflush(childLogFile);
      spin = (packet.data[0] != SERVER_OFF_COMMAND);
    }
  }

  exit(0);
}


//
// client.c
//
// Andrew Keesler
//
// Created February 6, 2015
//
// Client test application.
//

#include <stdlib.h>

#include "snet.h"
#include "sdl-types.h"
#include "phy.h" // sdlPhyTransmit()
#include "mac.h" // sdlPacketToFlatBuffer()

#include "client.h"

int nodeState = 0;

void sdlPhyButtonIsr(uint8_t button)
{
  
}

int main(void)
{
  SdlPacket packet;
  uint8_t serverCommand[SDL_PHY_SDU_MAX + 1];

  // Sleep.
  usleep(CLIENT_WAKEUP_DELAY);

  // Tell the server to turn off.
  packet.type = SDL_PACKET_TYPE_DATA;
  packet.sequence = 0xABCD;
  packet.source = 0x01234567;
  packet.destination = 0xFFFFFFFF;
  packet.dataLength = 1;
  sdlPacketToFlatBuffer(&packet, serverCommand);
  sdlPhyTransmit(serverCommand, SDL_MAC_PDU_LEN + 1); // packet length

  // See ya.
  exit(0);
}

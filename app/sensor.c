//
// sensor.c
//
// Andrew Keesler
//
// July 19, 2015
//
// Sensor test app.
//

#include "sdl.h"

#include "sensor-sink.h"
#include "sensor.h"

#include "snet/src/child/child-main.h" // snetChildMain
#include "snet/src/child/child-log.h"  // snetChildLogPrintf()

#include "src/plat/led.h" // sdlPlatLedSet()

#include <assert.h> // assert()
#include <unistd.h> // getpid()
#include <stdlib.h> // exit()
#include <string.h> // memcpy()
#include <stdarg.h> // va_list, va_start(), va_arg(), va_end()
#include <stdio.h>  // printf()

// -----------------------------------------------------------------------------
// Internal State

#define STATE_ADVERTISE (0x00)
#define STATE_REPORT    (0x01)
static uint8_t state = STATE_ADVERTISE;

static SdlAddress sinkAddress = SDL_MAC_ADDRESS_BROADCAST;

// -----------------------------------------------------------------------------
// Internal Declarations

#define note(...) printf(__VA_ARGS__), fflush(0);

static void loop(void);
static void unconnectedTask(void);

// -----------------------------------------------------------------------------
// Main

int main(int argc, char *argv[])
{
  // Initialize SDL functionality.
  assert(sdlMacInit(getpid()) == SDL_SUCCESS);

  loop();

  exit(0);
}

// -----------------------------------------------------------------------------
// Internal Definitions

static void loop(void)
{
  while (1) {
    switch (state) {
    case STATE_ADVERTISE:
      sdlPlatLedSet(SENSOR_ADVERTISE_LED);
      unconnectedTask();
      break;

    default:
      ; // eh?
    }
  }
}

static void unconnectedTask(void)
{
  uint8_t data[SENSOR_SINK_ADVERTISEMENT_PAYLOAD_SIZE];
  uint32_t key = SENSOR_SINK_ADVERTISEMENT_KEY;
  uint16_t prof = SENSOR_PROFILE;
  SdlStatus status;
  SdlPacket packet;

  // First, we sleep. This is to keep us from hammering the network interface
  // and the network medium. Furthermore, it gives a sink a chance
  // to respond to us.
  usleep(SENSOR_UNCONNECTED_DUTY_CYCLE_US);

  // If we received something...
  if (sdlMacReceive(&packet) == SDL_SUCCESS) {
    // ...then it might be an advertisement response, so decrypt it.
    sensorSinkDecrypt(packet.data, packet.dataLength, SENSOR_SINK_CRYPTO_KEY);
    // If the packet was a successfull advertisement response...
    if ((packet.data[SENSOR_SINK_COMMAND_INDEX]
         == SENSOR_SINK_ADVERTISEMENT_RESPONSE)
        && (packet.data[SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_INDEX]
            == SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_SUCCESS)) {
      // ...then we have found our sink!
      sinkAddress = packet.source;
      sensorSinkPrintf("Sensor: Connected with sink: 0x%08X", sinkAddress);
    }
  } else {
    // Else, we send out another advertisement.

    // Build our sensor advertisement (big-endian).
    data[SENSOR_SINK_COMMAND_INDEX] = SENSOR_SINK_ADVERTISEMENT;

    data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 0] = ((key & 0xFF000000) >> 0x18);
    data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 1] = ((key & 0x00FF0000) >> 0x10);
    data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 2] = ((key & 0x0000FF00) >> 0x08);
    data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 3] = ((key & 0x000000FF) >> 0x00);

    data[SENSOR_SINK_ADVERTISEMENT_PROFILE_INDEX + 0] = ((prof & 0xFF00) >> 0x08);
    data[SENSOR_SINK_ADVERTISEMENT_PROFILE_INDEX + 1] = ((prof & 0x00FF) >> 0x00);

    // Encrypt the advertisement.
    sensorSinkEncrypt(data, sizeof(data), SENSOR_SINK_CRYPTO_KEY);

    // Transmit.
    status = sdlMacTransmit(SDL_PACKET_TYPE_DATA,
                            SDL_MAC_ADDRESS_BROADCAST,
                            data,
                            sizeof(data));
    sensorSinkPrintf("Sensor: Broadcast advertisement: 0x%02X", status);
  }
}

// -----------------------------------------------------------------------------
// Callbacks

void sdlPhyButtonIsr(uint8_t button)
{
}

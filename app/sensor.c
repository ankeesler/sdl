//
// sensor.c
//
// Andrew Keesler
//
// July 19, 2015
//
// Sensor test app.
//

#include "inc/sdl.h"

#include "sensor-sink.h"
#include "sensor.h"

#include "src/snet/inc/snet-child.h"

#include <assert.h> // assert()
#include <unistd.h> // getpid()
#include <stdlib.h> // exit()
#include <string.h> // memcpy(), strcmp()
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
static void reportTask(void);

static void serialTick(void);

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

    case STATE_REPORT:
      sdlPlatLedClear(SENSOR_ADVERTISE_LED);
      reportTask();
      break;

    default:
      ; // eh?
    }

    serialTick();
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
      state = STATE_REPORT;
      sensorSinkPrintf("Connected with sink: 0x%08X.\n", sinkAddress);
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
    sensorSinkPrintf("Broadcast advertisement: 0x%02X.\n", status);
  }
}

static void reportTask(void)
{
  uint8_t payload[SENSOR_REPORT_DATA_MAX_SIZE], *finger, length;
  SdlStatus status;
  uint16_t data = 0x1234;

  // TODO: make this configurable by the sink.
  usleep(SENSOR_REPORT_DUTY_CYCLE_US);

  finger = payload;
  *finger++ = SENSOR_SINK_DATA_COMMAND;
  *finger++ = SENSOR_SINK_DATA_TYPE_COUNT;
  *finger++ = (data & 0x00FF) >> 0x00;
  *finger++ = (data & 0xFF00) >> 0x08;

  length = finger - &payload[0];
  sensorSinkEncrypt(payload, length, SENSOR_SINK_CRYPTO_KEY);

  status = sdlMacTransmit(SDL_PACKET_TYPE_DATA,
                          sinkAddress,
                          payload,
                          length);
  sensorSinkPrintf("Report data: 0x%04X.\n", data);
}

static void serialTick(void)
{
  uint8_t command[16];
  uint8_t commandIndex;

  for (commandIndex = 0;
       (commandIndex < sizeof(command)
        && sdlPlatSerialRead(command + commandIndex) == SDL_SUCCESS);
       commandIndex ++) { }

  if (commandIndex <= 1) {
    return;
  }

  sensorSinkPrintf("Received serial command '%s'.\n", command);

  if (strcmp((char *)command, "status") == 0) {
    sensorSinkPrintf("Status: 0x%02X.\n", state);
  }
}

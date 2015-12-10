//
// sink.c
//
// Andrew Keesler
//
// July 23, 2015
//
// Sink test app.
//

#include "sdl.h"

#include "sensor-sink.h"
#include "sink.h"

#include "snet/src/child/child-main.h" // snetChildMain

#include <assert.h> // assert()
#include <unistd.h> // getpid()
#include <stdlib.h> // exit()
#include <string.h> // strlen(), memset()
#include <stdio.h>  // printf()

// -----------------------------------------------------------------------------
// Internal State

#define STATE_LISTEN_FOR_ADVERTISEMENT (0x00)
static uint8_t state = STATE_LISTEN_FOR_ADVERTISEMENT;

typedef struct {
  SdlAddress address;
  uint16_t profile;
  uint8_t mask;
} SensorData;
#define SENSOR_DATA_MASK_USED (1 << 0)
static SensorData sensorList[SINK_MAX_SENSOR_COUNT];

// -----------------------------------------------------------------------------
// Internal Declarations

#define note(...) printf(__VA_ARGS__), fflush(0);

static void initSensorList(void);
static void loop(void);
static void listenForAdvertisement(void);
static void processData(SdlPacket *packet);

static void processAdvertisement(SdlPacket *advertisement);

// This returns 0xFF if it couldn't find any sensor with that address.
// If the address is SDL_MAC_ADDRESS_BROADCAST, then this will return
// the first available sensor index.
static uint8_t findSensorForAddress(SdlAddress address);

// -----------------------------------------------------------------------------
// Main

int main(int argc, char *argv[])
{
  // Initialize SDL functionality.
  assert(sdlMacInit(getpid()) == SDL_SUCCESS);

  // Initialize the sensor list.
  initSensorList();

  loop();

  exit(0);
}

static void initSensorList(void)
{
  uint8_t i;

  for (i = 0; i < SINK_MAX_SENSOR_COUNT; i ++) {
    sensorList[i].mask = 0;
    sdlPlatLedClear(i);
  }
}

static void loop(void)
{
  while (1) {
    switch (state) {
    case STATE_LISTEN_FOR_ADVERTISEMENT:
      listenForAdvertisement();
      break;
      
    default:
      ; // eh?
    }
  }
}

static void listenForAdvertisement(void)
{
  SdlPacket packet;

  // If there is a packet to receive...
  if (sdlMacReceive(&packet) == SDL_SUCCESS) {
    // ...we first must decrypt it.
    sensorSinkDecrypt(packet.data, packet.dataLength, SENSOR_SINK_CRYPTO_KEY);
    // If it is an advertisement...
    if (packet.data[SENSOR_SINK_COMMAND_INDEX] == SENSOR_SINK_ADVERTISEMENT) {
      // ...then process the advertisement...
      processAdvertisement(&packet);
    } else if (packet.data[SENSOR_SINK_COMMAND_INDEX] == SENSOR_SINK_DATA_COMMAND) {
      // ...and if it is data, then process that.
      processData(&packet);
    }
  }
}

static void processAdvertisement(SdlPacket *advertisement)
{
  uint32_t receivedKey = 0;
  uint16_t profile = 0;
  uint8_t responseStatus = SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_FAILURE;
  uint8_t sensorIndex = 0xFF;
  uint8_t data[SENSOR_SINK_ADVERTISEMENT_RESPONSE_PAYLOAD_SIZE];

  sensorSinkPrintf("Sink: Received advertisement from: 0x%08X.\n",
                   advertisement->source);

  // Don't reconnect with any address that you already are connected with.
  if (findSensorForAddress(advertisement->source) != 0xFF) {
    return;
  }
  
  // Remember, big-endian.

  // Get the received advertisement key.
  receivedKey |= advertisement->data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 0];
  receivedKey <<= 0x08;
  receivedKey |= advertisement->data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 1];
  receivedKey <<= 0x08;
  receivedKey |= advertisement->data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 2];
  receivedKey <<= 0x08;
  receivedKey |= advertisement->data[SENSOR_SINK_ADVERTISEMENT_KEY_INDEX + 3];

  // Get the profile.
  profile |= advertisement->data[SENSOR_SINK_ADVERTISEMENT_PROFILE_INDEX + 0];
  profile <<= 0x08;
  profile |= advertisement->data[SENSOR_SINK_ADVERTISEMENT_PROFILE_INDEX + 1];

  // If the received key matches the expected key...
  if (receivedKey == SENSOR_SINK_ADVERTISEMENT_KEY) {
    sensorIndex = findSensorForAddress(SDL_MAC_ADDRESS_BROADCAST);
    // ...and there is an available sensor data...
    if (sensorIndex != 0xFF) {
      // ...then we successfully connected with the sensor!
      sensorList[sensorIndex].address = advertisement->source;
      sensorList[sensorIndex].profile = profile;
      sensorList[sensorIndex].mask |= SENSOR_DATA_MASK_USED;
      responseStatus = SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_SUCCESS;
      sdlPlatLedSet(sensorIndex);
      sensorSinkPrintf("Sink: Connected with sensor for profile: 0x%04X.\n", profile);
    }
  }

  // Send the response.
  data[SENSOR_SINK_COMMAND_INDEX] = SENSOR_SINK_ADVERTISEMENT_RESPONSE;
  data[SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_INDEX] = responseStatus;
  sensorSinkEncrypt(data, sizeof(data), SENSOR_SINK_CRYPTO_KEY);
  sdlMacTransmit(SDL_PACKET_TYPE_DATA,
                 advertisement->source,
                 data,
                 sizeof(data));
}

static void processData(SdlPacket *packet)
{
  sensorSinkPrintf("Sink: Receive data from 0x%08X.\n", packet->source);
}

// This returns 0xFF if it couldn't find any sensor with that address.
// If the address is SDL_MAC_ADDRESS_BROADCAST, then this will return
// the first available sensor index.
static uint8_t findSensorForAddress(SdlAddress address)
{
  bool used;
  uint8_t i;

  for (i = 0; i < SINK_MAX_SENSOR_COUNT; i ++) {
    used = ((sensorList[i].mask & SENSOR_DATA_MASK_USED) == SENSOR_DATA_MASK_USED);
    if ((address == SDL_MAC_ADDRESS_BROADCAST && !used)
        || (address == sensorList[i].address && used)) {
      return i;
    }
  }

  return 0xFF;
}

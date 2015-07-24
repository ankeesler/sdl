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

#include <assert.h> // assert()
#include <unistd.h> // getpid()
#include <stdlib.h> // exit()
#include <string.h> // strlen(), memset()
#include <stdarg.h> // va_list, va_start(), va_arg(), va_end()
#include <stdio.h>  // vsnprint()

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

static void loop(void);
static void listenForAdvertisement(void);

static void processAdvertisement(SdlPacket *advertisement);

// This returns 0xFF if it couldn't find any sensor with that address.
// If the address is SDL_MAC_ADDRESS_BROADCAST, then this will return
// the first available sensor index.
static uint8_t findSensorForAddress(SdlAddress address);

static void decryptData(uint8_t *data, uint8_t length);

static void sinkPrint(const char *format, ...);

// -----------------------------------------------------------------------------
// Main

int main(void)
{
  uint8_t i;

  // Initialize SDL functionality.
  assert(sdlMacInit(getpid()) == SDL_SUCCESS);

  // Initialize the sensor list.
  for (i = 0; i < SINK_MAX_SENSOR_COUNT; i ++) {
    memset(sensorList, 0, sizeof(sensorList));
  }

  loop();

  exit(0);
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
    decryptData(packet.data, packet.dataLength);
    // If it is an advertisement...
    if (packet.data[SENSOR_SINK_COMMAND_INDEX] == SENSOR_SINK_ADVERTISEMENT) {
      // ...then process the advertisement.
      sinkPrint("Sink: Received advertisement from: 0x%08X", packet.source);
      processAdvertisement(&packet);
    }
  }
}

static void processAdvertisement(SdlPacket *advertisement)
{
  uint32_t receivedKey = 0;
  uint16_t profile = 0;
  uint8_t responseStatus = SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_FAILURE;
  uint8_t sensorIndex = 0xFF;
  
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
      sinkPrint("Sink: Connected with sensor for profile: 0x%04X", profile);
    }
  }
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

static void decryptData(uint8_t *data, uint8_t length)
{
  const char *key = SENSOR_SINK_CRYPTO_KEY;
  uint8_t dataI, keyI, keyLength;

  keyLength = strlen(key);

  // Only the highest of encryption standards.
  for (dataI = 0; dataI < length; dataI ++) {
    data[dataI] -= key[keyI];

    if (++keyI == keyLength) {
      keyI = 0;
    }
  }
}

static void sinkPrint(const char *format, ...)
{
  char data[UINT8_MAX];
  va_list args;

  va_start(args, format);

  vsnprintf(data, sizeof(data), format, args);

  sdlUartTransmit((uint8_t *)data, strlen(data));

  va_end(args);
}

// -----------------------------------------------------------------------------
// Callbacks

void sdlPhyButtonIsr(uint8_t button)
{
}

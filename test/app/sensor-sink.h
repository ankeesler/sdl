//
// sensor-sink.h
//
// Andrew Keesler
//
// July 19, 2015
//
// Sensor-sink scenario header.
//

// This sensor/sink scenario presents two simple devices communicating
// using an SDL link.

// In order to establish a connection with a sink, the sensor must
// successfully find and pair with a sink. To do this, it first issues
// a SENSOR_SINK_ADVERTISEMENT. When the sensor receives this command,
// it must respond with a SENSOR_SINK_ADVERTISEMENT_RESPONSE command. A status
// byte contained in the payload of this command with a value of 0x00
// indicates that the two devices are successfully paired.

#define SENSOR_SINK_COMMAND_INDEX (0)

#define SENSOR_SINK_ADVERTISEMENT          (0x0A)
#define SENSOR_SINK_ADVERTISEMENT_RESPONSE (0x8A)

#define SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_INDEX   (1)
#define SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_SUCCESS (0x00)
#define SENSOR_SINK_ADVERTISEMENT_RESPONSE_STATUS_FAILURE (0x01)

// In order for the sink to protect itself from evil sensors, it uses
// a very advanced form of encryption on its received and transmitted
// packets. Sensor devices wishing to establish a connection with a sink
// must encrypt their SENSOR_SINK_ADVERTISE command with the correct key
// so that the sink knows that it can be trusted.

#define SENSOR_SINK_CRYPTO_KEY "tuna"

#define SENSOR_SINK_ADVERTISEMENT_KEY_INDEX  (1)
#define SENSOR_SINK_ADVERTISEMENT_KEY_LENGTH (4)

#define SENSOR_SINK_ADVERTISEMENT_KEY (0xF00DF00D)

#define SENSOR_SINK_ADVERTISEMENT_PROFILE_INDEX  (5)
#define SENSOR_SINK_ADVERTISEMENT_PROFILE_LENGTH (2)

#define SENSOR_SINK_ADVERTISEMENT_PROFILE_PIZZA (0xABCD)
#define SENSOR_SINK_ADVERTISEMENT_PROFILE_FISH  (0x1234)

#define SENSOR_SINK_ADVERTISEMENT_PAYLOAD_SIZE          (7)
#define SENSOR_SINK_ADVERTISEMENT_RESPONSE_PAYLOAD_SIZE (2)

// After the two devices are successfully connected, the sensor can send
// SENSOR_SINK_DATA_COMMAND to the sink. Each of these commands has a
// certain type of data associated with it, and payload formats and sizes
// vary based on this type of data.

#define SENSOR_SINK_DATA_COMMAND (0x0C)

#define SENSOR_SINK_DATA_TYPE_INDEX  (1)
#define SENSOR_SINK_DATA_TYPE_LENGTH (1)

#define SENSOR_SINK_DATA_TYPE_COUNT        (0x12)
#define SENSOR_SINK_DATA_TYPE_COUNT_LENGTH (2)

#define SENSOR_SINK_DATA_TYPE_TIME        (0x34)
#define SENSOR_SINK_DATA_TYPE_TIME_LENGTH (4)

// Common routines to the Sensor/Sink scenario.
void sensorSinkEncryptOrDecrypt(uint8_t *data, uint8_t length, const char *key);
#define sensorSinkEncrypt(data, length, key)    \
  sensorSinkEncryptOrDecrypt(data, length, key)
#define sensorSinkDecrypt(data, length, key)    \
  sensorSinkEncryptOrDecrypt(data, length, key)

void sensorSinkPrintf(const char *format, ...);

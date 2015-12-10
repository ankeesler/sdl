//
// sensor-sink-test.c
//
// Andrew Keesler
//
// July 19, 2015
//
// Sensor sink test.
//

#include <unit-test.h>

#include "snet/src/parent/network.h" // snetNetworkLedRead()
#include "snet/src/parent/expect.h"  // snetExpect()

#include "sdl-test-util.h"

#include "app/sensor.h"

#include <unistd.h> // usleep()

static int sensorSinkTest(void)
{
  expectEquals(snetNetworkSize(), 0);

  expectEquals(snetNetworkAddNode("sensor", SENSOR_IMAGE), SNET_ERRNO_SUCCESS);
  expectEquals(snetNetworkSize(), 1);

  expectEquals(snetNetworkAddNode("sink", SINK_IMAGE), SNET_ERRNO_SUCCESS);
  expectEquals(snetNetworkSize(), 2);

  // The sensor should have its advertise LED on.
  bool led = false;
  usleep(CHILD_TIMEOUT_USEC);
  expectEquals(snetNetworkLedRead("sensor", SENSOR_ADVERTISE_LED, &led),
               SNET_ERRNO_SUCCESS);
  expect(led);

  // Sleep so that we make sure the sensor sends out at least one advertisement.
  usleep(SENSOR_UNCONNECTED_DUTY_CYCLE_US);

  // The sink should turn on an led when it gets connected to a sensor.
  // Since this is the first sensor, the led should be 0.
  led = false;
  usleep(CHILD_TIMEOUT_USEC);
  expectEquals(snetNetworkLedRead("sink", 0, &led), SNET_ERRNO_SUCCESS);
  expect(led);

  // The sensor should turn off its advertising LED since it is connected.
  usleep(CHILD_TIMEOUT_USEC);
  expectEquals(snetNetworkLedRead("sensor", SENSOR_ADVERTISE_LED, &led),
               SNET_ERRNO_SUCCESS);
  expect(!led);

  // The sensor should report at some point.
  expectEquals(snetExpect("sensor",
                          "Report data: 0x[A-F0-9]{4}.*",
                          SENSOR_REPORT_DUTY_CYCLE_US * 2),
               SNET_ERRNO_SUCCESS);

  // The sink should receive it.
  expectEquals(snetExpect("sink",
                          "Sink: Receive data from 0x[A-F0-9]{8}.*",
                          SENSOR_REPORT_DUTY_CYCLE_US * 2),
               SNET_ERRNO_SUCCESS);

  expectEquals(snetNetworkRemoveNode("sensor"), SNET_ERRNO_SUCCESS);
  expectEquals(snetNetworkSize(), 1);

  expectEquals(snetNetworkRemoveNode("sink"), SNET_ERRNO_SUCCESS);
  expectEquals(snetNetworkSize(), 0);

  return 0;
}

int main(int argc, char *argv[])
{
  announce();

  setFailureHandler(snetNetworkClear);
  setVerbose((argc > 1 && !strcmp(argv[1], "-v")));

  run(sensorSinkTest);

  return 0;
}

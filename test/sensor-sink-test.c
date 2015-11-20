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

#include "sdl-test-util.h"

#include "app/sensor.h"

#include <unistd.h> // usleep()

static void failureHandler(void)
{
}

static int sensorSinkTest(void)
{
  expectEquals(snetNetworkSize(), 0);

  expectEquals(snetNetworkAddNode("sensor", SENSOR_IMAGE), 0);
  expectEquals(snetNetworkSize(), 1);

  expectEquals(snetNetworkAddNode("sink", SINK_IMAGE), 0);
  expectEquals(snetNetworkSize(), 2);

  // The sensor should have its advertise LED on.
  bool led = false;
  expectEquals(snetNetworkLedRead("sensor", SENSOR_ADVERTISE_LED, &led), 0);
  usleep(CHILD_TIMEOUT_USEC);
  expect(led);

  expectEquals(snetNetworkRemoveNode("sensor"), 0);
  expectEquals(snetNetworkSize(), 1);

  expectEquals(snetNetworkRemoveNode("sink"), 0);
  expectEquals(snetNetworkSize(), 0);

  return 0;
}

int main(int argc, char *argv[])
{
  announce();

  setFailureHandler(failureHandler);
  setVerbose((argc > 1 && !strcmp(argv[1], "-v")));

  run(sensorSinkTest);

  return 0;
}

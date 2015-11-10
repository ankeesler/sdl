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

#include "snet/src/parent/network.h"

#include "sdl-test-util.h"

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

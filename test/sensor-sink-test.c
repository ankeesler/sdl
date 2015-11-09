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

static int sensorTest(void)
{
  expectEquals(snetNetworkAddNode("sensor", SENSOR_IMAGE), 0);
  expectEquals(snetNetworkSize(), 1);

  expectEquals(snetNetworkRemoveNode("sensor"), 0);

  return 0;
}

int main(int argc, char *argv[])
{
  announce();

  setFailureHandler(failureHandler);
  setVerbose((argc > 1 && !strcmp(argv[1], "-v")));

  run(sensorTest);

  return 0;
}

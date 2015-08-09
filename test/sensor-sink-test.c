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

#include "snet-expect.h"

#include "sdl-test-util.h"
#include "test/app/sensor.h"

#include <assert.h> // assert()

static FILE *logFile = NULL;

static void initLogFile(void)
{
  assert((logFile = fopen(__FILE__ ".log", "w")));
}

static void deinitLogFile(void)
{
  fclose(logFile);
}

SnetNode *sensor = NULL, *sink = NULL;

static void failureHandler(void)
{
  if (sink)   kill(sink->pid, SIGTERM);
  if (sensor) kill(sensor->pid, SIGTERM);

  while (wait(NULL) != -1) ;

  deinitLogFile();
}

static int sensorSinkTest(void)
{
  // Initialize the expect framework.
  expectEquals(snetExpectInit(logFile), SDL_SUCCESS);

  // Create the sensor.
  expect((int)(sensor = snetNodeMake("build/sensor/sensor", "sensor")));

  // Boot the sensor.
  expectEquals(snetNodeStart(sensor), SDL_SUCCESS);

  // The sensor should start advertising.
  expect(snetExpect(sensor,
                    "Sensor: Broadcast advertisement: 0x00",
                    SENSOR_UNCONNECTED_DUTY_CYCLE_US << 2));

  // Create the sink.
  expect((int)(sink = snetNodeMake("build/sink/sink", "sink")));

  // Boot the sink.
  expectEquals(snetNodeStart(sink), SDL_SUCCESS);

  // The sink should receive the sink's advertisement.
  expect(snetExpect(sink,
                    "Sink: Received advertisement from: 0x........",
                    SENSOR_UNCONNECTED_DUTY_CYCLE_US << 2));

  // The sink should successfully connect with the sensor.
  expect(snetExpect(sink,
                    "Sink: Connected with sensor for profile: 0xABCD",
                    DEFAULT_TIMEOUT_US << 3));

  // FIXME: the sink should receive a successfull advertisement response.
  /*
  expect(snetExpect(sensor,
                    "Sensor: Connected with sink: 0x........",
                    SENSOR_UNCONNECTED_DUTY_CYCLE_US << 2));
  */

  // Tear down the network.
  expectEquals(snetManagementDeinit(), 2);
  expectEquals(snetManagementSize(), 0);

  return 0;
}

int main(int argc, char *argv[])
{
  announce();

  setFailureHandler(failureHandler);
  setVerbose((argc > 1 && !strcmp(argv[1], "-v")));

  initLogFile();

  run(sensorSinkTest);

  return 0;
}

//
// snet-expect-test.c
//
// Andrew Keesler
//
// July 18, 2015
//
// Unit tests for SNET expect framework.
//

#include <unit-test.h>

#include "snet-expect.h"

#include "sdl-test-util.h"
#include "test/app/server.h"

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

static SnetNode *server1 = NULL, *server2 = NULL;

static void failureHandler(void)
{
  if (server1) kill(server1->pid, SIGTERM);
  if (server2) kill(server2->pid, SIGTERM);

  while (wait(NULL) != -1) ;

  deinitLogFile();
}

static int expectTest(void)
{
  server1 = server2 = NULL;

  // Initialize the expect framework.
  expectEquals(snetExpectInit(logFile), SDL_SUCCESS);

  // Add two servers.
  expect((int)(server1 = snetNodeMake("build/server/server", "server1")));
  expect((int)(server2 = snetNodeMake("build/server/server", "server2")));
  expectEquals(snetNodeStart(server1), SDL_SUCCESS);
  expectEquals(snetNodeStart(server2), SDL_SUCCESS);
  expect(RUNNING(server1));
  expect(RUNNING(server2));

  // Push button1. It should print some stuff.
  expectEquals(snetNodeCommand(server1, BUTTON, SERVER_UART_BUTTON1),
               SDL_SUCCESS);
  expect(snetExpect(server1, SERVER_UART_BUTTON1_STRING, DEFAULT_TIMEOUT_US));

  // Don't do anything. The server shouldn't print anything, so we
  // shouldn't find a match. Don't wait for very long so the test
  // takes a shorter amount of time. We may want to change this.
  expect(!snetExpect(server1, SERVER_UART_BUTTON1_STRING, 1000));
  expect(!snetExpect(server1, ".*", 1000));

  // Push button1 again. It should print some stuff. The wildcard
  // regex should work.
  expectEquals(snetNodeCommand(server1, BUTTON, SERVER_UART_BUTTON1),
               SDL_SUCCESS);
  expect(snetExpect(server1, ".*", DEFAULT_TIMEOUT_US));

  // Push button1 again. The expect for server2 should not work.
  // Again, wait for only a millisecond so that the test is shorter.
  expectEquals(snetNodeCommand(server1, BUTTON, SERVER_UART_BUTTON1),
               SDL_SUCCESS);
  expect(!snetExpect(server2, SERVER_UART_BUTTON1_STRING, 1000));

  // We should not catch something on the uart that has been printed
  // in the past!
  expectEquals(snetNodeCommand(server1, BUTTON, SERVER_UART_BUTTON1),
               SDL_SUCCESS);
  usleep(SERVER_DUTY_CYCLE_US << 2);
  expect(!snetExpect(server1, SERVER_UART_BUTTON1_STRING, 1000));

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

  run(expectTest);

  return 0;
}

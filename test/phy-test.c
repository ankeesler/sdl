//
// phy-test.c
//
// Andrew Keesler
//
// May 22, 2015
//
// Test for PHY layer APIs.
//

#include <unit-test.h>
#include <signal.h>
#include <sys/wait.h>

#define __SNET_C__
#include "snet.h"
#include "sdl-types.h"

#include "sdl-test-util.h"
#include "nodes/client.h"
#include "nodes/server.h"

// Stubs.
SdlStatus sdlPhyTransmit(uint8_t *data, uint8_t length) { return SDL_SUCCESS; }

SnetNode *server = NULL;
SnetNode *client = NULL;

static void failureHandler(void)
{
  if (server) kill(server->pid, SIGTERM);
  if (client) kill(client->pid, SIGTERM);

  while (wait(NULL) != -1) ;
}

int transmitTest(void)
{
  // Make a server and a client.
  expect((int)(server = snetNodeMake("server", "build/server/server")));
  expect((int)(client = snetNodeMake("client", "build/client/client")));

  // Add them to the network.
  expect(!snetNodeStart(server));
  expect(RUNNING(server));
  expect(!snetNodeStart(client));
  expect(RUNNING(client));

  // Wait for the client to do its thang and tell the server to turn off.
  usleep(CLIENT_WAKEUP_DELAY * 2);

  // After a duty cycle, the server should shut down.
  usleep(SERVER_DUTY_CYCLE_US);
  //expect(!RUNNING(server));

  // Tear the network down.
  expect(snetManagementDeinit() > 0);
  expect(!snetManagementSize());

  return 0;
}

int main(void)
{
  announce();

  setFailureHandler(failureHandler);

  run(transmitTest);

  return 0;
}

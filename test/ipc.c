//
// ipc.c
//
// Andrew Keesler
//
// February 1, 2015
//
// Interprocess communication test.

#include <unit-test.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "sdl.h"

unsigned char data[] = {1, 2, 3};
unsigned char length = 3;

static void childTx(void)
{
  exit(sdlTransmit(data, length));
}

static void childRx(void)
{
  // Wait for the childTx process to transmit.
  usleep(5000);
  data[0] = data[1] = data[2] = 0;
  sdlReceive(data, length);
  exit(data[0] + (data[1] * data[2]));
}

int syncOneWayTest(void)
{
  pid_t pidTx, pidRx;
  int txReturn, rxReturn;
  
  if ((pidTx = fork()) == -1) { // Failure.
    return 1;
  } else if (pidTx) { // Parent.
    if ((pidRx = fork()) == -1) { // Failure.
      return 2;
    } else if (pidRx) { // Parent.
      ;
    } else { // Child Rx.
      childRx();
    }
  } else { // Child Tx.
    childTx();
  }
  
  // Make the wait() calls first so that we know the processes finished.
  wait(&txReturn);
  wait(&rxReturn);
  expect(WEXITSTATUS(txReturn) == 0);
  expect(WEXITSTATUS(rxReturn) == (1 + (2 * 3)));
  
  return 0;
}

int main(void)
{
  announce();
  run(syncOneWayTest);
  return 0;
}
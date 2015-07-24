//
// snet-expect.h
//
// Andrew Keesler
//
// July 18, 2015
//
// SNET expect framework.
//

#include "snet-expect.h"

#include <string.h>   // memcpy
#include <sys/time.h> // timeval
#include <regex.h>

#ifdef SNET_TEST
  #include <unit-test.h>
#endif

// State for received uart stuff.
static uint8_t uartRxData[UINT8_MAX], uartRxDataLength;
static const char *uartRxNode = NULL;
static struct timeval uartRxTime;
#define uartRxClear()      (uartRxNode = NULL)
#define uartRxDataExists() (uartRxNode)
#define uartRxData()       ((const char *)uartRxData)
#define uartRxDataLength() (uartRxDataLength)
#define uartRxNode()       (uartRxNode)
#define uartRxTime()       (uartRxTime)

// The one uart isr to rule them all.
static void uartIsr(const char *node, uint8_t *data, uint8_t length)
{
  gettimeofday(&uartRxTime, NULL); // don't care about timezone

  memcpy(uartRxData, data, length);
  uartRxDataLength = length;
  uartRxNode = node;

  // TODO: write data to a file!
}

static inline void getStopTime(struct timeval *start,
                               struct timeval *stop,
                               uint32_t timeoutUS)
{
  struct timeval add;

  timerclear(&add);
  while (timeoutUS > 1000000) {
    add.tv_sec ++;
    timeoutUS -= 1000000;
  }
  add.tv_usec = timeoutUS;

  timeradd(start, &add, stop);
}

SdlStatus snetExpectInit(void)
{
  SdlStatus status = SDL_FATAL;

  // If the SNET UART ISR is already in use, then we cannot setup
  // our framework.
  if (!snetNodeUartIsr(NULL)) {
    snetNodeUartIsr(uartIsr);
    status = SDL_SUCCESS;
    snetManagementInit();
  }

  return status;
}

bool snetExpect(SnetNode *node, const char *regexString, uint32_t timeoutUS)
{
  struct timeval stop, now;
  regex_t regex;
  regmatch_t regmatch;
  int regexStatus;

  // Clear old UART data so we don't catch something that is old.
  uartRxClear();

  // Compile the regex, returning failure if there was an issue.
  regexStatus = regcomp(&regex, regexString, REG_EXTENDED);

  if (regexStatus == 0) {
    // Set the stop time.
    gettimeofday(&now, NULL); // don't care about time zone
    getStopTime(&now, &stop, timeoutUS);

    // Initialize the regex status.
    regexStatus = REG_NOMATCH;

    // While we haven't reached our timeout, try to find the regex.
    do {
      // If the UART received stuff that is for this node...
      if (uartRxDataExists() && !strcmp(node->name, uartRxNode())) {
        // ...try the regex on it and see if it matches.
        regexStatus = regexec(&regex,
                              uartRxData(),
                              1,         // try to match one
                              &regmatch, // capture group for now?
                              0); // TODO: any flags here?

        // Make sure to clear the uart data.
        uartRxClear();
      }

      gettimeofday(&now, NULL);   // don't care about timezone
    } while (regexStatus == REG_NOMATCH && timercmp(&now, &stop, <));
  }

  // TODO: report regex error? Use regerror?

  regfree(&regex);

  return (regexStatus == 0);
}

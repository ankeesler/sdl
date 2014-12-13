//
// sdl-log.c
//
// Andrew Keesler
//
// December 13, 2014
//
// Log the stuff that comes into the network.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <limits.h>

#include "sdl-log.h"

#ifndef SDL_LOG
// Then stubs.
int sdlLogInit() { return 0; }
int sdlLogOn() { return 0; }
int sdlLogDump() { return 0; }
int sdlLogTx(unsigned char *bytes, int length) { return 0; }
int sdlLogRx(unsigned char *bytes, int length) { return 0; }
#else //SDL_LOG

#ifdef SDL_LOG_FILE
#define FILENAME SDL_LOG_FILE
#else
#define FILENAME ""
#endif

typedef struct LogEvent {
  struct timeval time;
  unsigned char *data;
  int length;
  int isTx; // if not then rx
  struct LogEvent *next;
} LogEvent;

// Linked list.
static LogEvent *head = NULL, *tail = NULL;

static struct timeval startTime;

int sdlLogInit()
{
  // Start clock.
  gettimeofday(&startTime, NULL);

  head = tail = NULL;

  return 0;
}

int sdlLogOn() { return 1; }

int sdlLogDump()
{
  FILE *stream = NULL;
  const char *possibleFilename = FILENAME;
  int i;

  // This is gonna be a doozy.

  // Find out if the stream is a file or stdout.
  if (!strlen(possibleFilename)) {
    stream = stdout;
  } else if (!(stream = fopen(possibleFilename, "w+"))) {
    return 1;
  }

  // Now just dump.
  fprintf(stream, "CAPTURE\n{\n");
  while (head) {
    // Haha a double amirite.
    double timeS
      = ((head->time.tv_sec - startTime.tv_sec)
         + (((head->time.tv_usec > startTime.tv_usec
              ? head->time.tv_usec - startTime.tv_usec
              : LONG_MAX - startTime.tv_usec - head->time.tv_usec)
             / 1000000.0)));
    fprintf(stream, "(%lf) %s\n  [", timeS, (head->isTx ? "TX" : "RX"));
    for (i = 0; i < head->length-1; i ++)
      fprintf(stream, "0x%02X, ", head->data[i]);
    fprintf(stream, "0x%02X]\n", head->data[i]);

    head = head->next;
  }
  fprintf(stream, "}\n");

  fflush(stream);

  return fclose(stream);
}

static int append(unsigned char *bytes, int length, int isTx)
{
  // Create event.
  LogEvent *event = (LogEvent *)malloc(sizeof(LogEvent));
  gettimeofday(&(event->time), NULL);
  event->data = (unsigned char *)malloc(sizeof(unsigned char) * length);
  memcpy(event->data, bytes, length);
  event->length = length;
  event->isTx = isTx;
  event->next = NULL;

  // Add to list.
  if (!head)
    head = event;
  if (tail)
    tail->next = event;
  tail = event;

  return 0;
}

int sdlLogTx(unsigned char *bytes, int length)
{
  return append(bytes, length, 1);
}

int sdlLogRx(unsigned char *bytes, int length)
{
  return append(bytes, length, 0);
}

#endif //SDL_LOG

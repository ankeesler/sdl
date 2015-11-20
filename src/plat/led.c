//
// led.c
//
// Andrew Keesler
//
// Monday, November 16, 2015
//
// Simulated LED driver.
//

#include <signal.h> // sig_atomic_t

#include "nvic.h"
#include "assert.h" // sdlPlatAssert()

#include "led.h"

volatile sig_atomic_t leds[SDL_LED_COUNT];

void nvicLedTouchedIsr(uint8_t led, bool on)
{
  sdlPlatAssert(led < SDL_LED_COUNT);
  leds[led] = on;
}

void sdlPlatLedSet(uint8_t led)
{
  sdlPlatAssert(led < SDL_LED_COUNT);
  leds[led] = true;
}

void sdlPlatLedClear(uint8_t led)
{
  sdlPlatAssert(led < SDL_LED_COUNT);
  leds[led] = false;
}

bool sdlPlatLedRead(uint8_t led)
{
  sdlPlatAssert(led < SDL_LED_COUNT);
  return (bool)leds[led];
}

//
// led.h
//
// Andrew Keesler
//
// Monday, November 16, 2015
//
// Simulated LED driver.
//

#include <stdint.h>
#include <stdbool.h>

#define SDL_LED_COUNT (3)

// Set/clear an LED.
void sdlPlatLedSet(uint8_t led);
void sdlPlatLedClear(uint8_t led);

// Read whether an LED is on or off.
bool sdlPlatLedRead(uint8_t led);


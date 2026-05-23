#ifndef LAVA_LAMP_H
#define LAVA_LAMP_H

#include <Adafruit_NeoPixel.h>
#include "lava_colors.h"

// Call once in setup()
void lavaLampBegin();

// Call every frame in loop() with current price in dollars
void lavaLampUpdate(float priceDollars);

// Access the strip directly if needed (e.g. for custom animations)
Adafruit_NeoPixel& getLavaStrip();

#endif

#include "lava_lamp.h"
#include <math.h>

#define LED_PIN 6
#define NUM_LEDS 20

static Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel& getLavaStrip() { return strip; }

void lavaLampBegin() {
  strip.begin();
  strip.setBrightness(30);
  strip.show();
}

void lavaLampUpdate(float priceDollars) {
  LavaColor c = lavaColorForPrice(priceDollars);
  strip.setBrightness(c.brightness);

  static float phase = 0.0;
  static float cometPos = 0.0;

  if (c.speed > 1.0) {
    // COMET MODE — trailing comet with fading tail for urgent prices
    cometPos += 0.05 * c.speed;
    if (cometPos >= NUM_LEDS) cometPos -= NUM_LEDS;

    // Fade all pixels toward black (creates the trail)
    for (int i = 0; i < NUM_LEDS; i++) {
      uint32_t col = strip.getPixelColor(i);
      int r = (int)((col >> 16 & 0xFF) * 0.88);
      int g = (int)((col >> 8 & 0xFF) * 0.88);
      int b = (int)((col & 0xFF) * 0.88);
      strip.setPixelColor(i, strip.Color(r, g, b));
    }

    // Paint the comet head at full brightness
    int head = (int)cometPos % NUM_LEDS;
    strip.setPixelColor(head, strip.Color(c.r, c.g, c.b));
  } else {
    // GLOW MODE — soft floating blob for normal prices
    phase += 0.004;
    float glowPos = (NUM_LEDS / 2.0) + sin(phase) * (NUM_LEDS / 2.0 - 1.0);

    for (int i = 0; i < NUM_LEDS; i++) {
      float dist = fabs(glowPos - i);
      float spread = 4.0;
      float brightness = exp(-(dist * dist) / (2.0 * spread * spread));
      // No ambient floor — distant LEDs go fully off for contrast

      int r = (int)(c.r * brightness);
      int g = (int)(c.g * brightness);
      int b = (int)(c.b * brightness);
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
  }

  strip.show();
}
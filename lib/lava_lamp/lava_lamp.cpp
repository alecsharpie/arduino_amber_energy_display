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
  // Cache color lookup — price only changes every 30 minutes
  static float lastPrice = -1.0;
  static LavaColor c = {0, 255, 0, 18, 1.0};
  if (priceDollars != lastPrice) {
    c = lavaColorForPrice(priceDollars);
    strip.setBrightness(c.brightness);
    lastPrice = priceDollars;
  }

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
    phase += 0.007;
    float glowPos = (NUM_LEDS / 2.0) + sin(phase) * (NUM_LEDS / 2.0 - 1.0);

    for (int i = 0; i < NUM_LEDS; i++) {
      float dist = fabs(glowPos - i);
      float spread = 4.0;
      float brightness = exp(-(dist * dist) / (2.0 * spread * spread));

      // Cut off very dim pixels to avoid color shift from integer truncation
      // (e.g. yellow {255,150,0} at low brightness becomes reddish)
      if (brightness < 0.10) {
        strip.setPixelColor(i, 0);
      } else {
        int r = (int)(c.r * brightness);
        int g = (int)(c.g * brightness);
        int b = (int)(c.b * brightness);
        strip.setPixelColor(i, strip.Color(r, g, b));
      }
    }
  }

  strip.show();
}
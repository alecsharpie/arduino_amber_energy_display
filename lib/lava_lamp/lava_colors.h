#ifndef LAVA_COLORS_H
#define LAVA_COLORS_H

// Pure colour logic — no Arduino dependencies.
// Shared by both the main firmware and the desktop unit tests.

struct LavaColor {
  int r, g, b;
  int brightness;
  float speed;  // animation speed multiplier (1.0 = calm, higher = faster)
};

inline LavaColor lavaColorForPrice(float priceDollars) {
  // Round to nearest cent to match the Amber app's displayed price
  int priceCents = (int)(priceDollars * 100.0 + 0.5);

  if (priceCents < 27) {
    return {0, 255, 0, 18, 1.0};      // GREEN  — cheap, calm glow
  } else if (priceCents < 37) {
    return {255, 150, 0, 30, 1.0};    // YELLOW — moderate
  } else if (priceCents < 47) {
    return {255, 50, 0, 30, 1.0};     // ORANGE — getting pricey
  } else if (priceCents < 57) {
    return {255, 0, 0, 30, 1.0};      // RED    — expensive
  } else {
    return {255, 0, 0, 30, 8.0};      // RED    — price spike, fast comet!
  }
}

#endif

#include "traffic_leds.h"
#include <Arduino.h>

#define RED_PIN 5
#define YELLOW_PIN 3
#define GREEN_PIN 9
#define BRIGHTNESS 40

void trafficLedsBegin() {
  // PWM pins don't need pinMode, analogWrite handles it
}

void trafficLedsUpdate(float renewables) {
  int band = (renewables >= 50) ? 2 : (renewables >= 20) ? 1 : 0;
  static int lastBand = -1;
  if (band == lastBand) return;
  lastBand = band;

  analogWrite(RED_PIN, 0);
  analogWrite(YELLOW_PIN, 0);
  analogWrite(GREEN_PIN, 0);

  if (band == 2) analogWrite(GREEN_PIN, BRIGHTNESS);
  else if (band == 1) analogWrite(YELLOW_PIN, BRIGHTNESS);
  else analogWrite(RED_PIN, BRIGHTNESS);
}
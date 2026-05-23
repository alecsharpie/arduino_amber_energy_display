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
  analogWrite(RED_PIN, 0);
  analogWrite(YELLOW_PIN, 0);
  analogWrite(GREEN_PIN, 0);

  if (renewables >= 50) {
    analogWrite(GREEN_PIN, BRIGHTNESS);
  } else if (renewables >= 20) {
    analogWrite(YELLOW_PIN, BRIGHTNESS);
  } else {
    analogWrite(RED_PIN, BRIGHTNESS);
  }
}
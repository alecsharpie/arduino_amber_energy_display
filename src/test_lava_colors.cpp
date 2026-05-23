// ============================================================
// Lava Lamp Color Integration Test
// ============================================================
// Upload with: pio run -e test_colors -t upload
// Then watch:  pio device monitor
//
// Cycles through test prices, holding each for 3 seconds.
// Compare what you SEE on the strip vs the expected color
// printed to Serial Monitor.
// ============================================================

#include <Arduino.h>
#include "lava_lamp.h"

struct TestCase {
  float price;
  const char* expectedColor;
};

TestCase tests[] = {
  {0.00, "GREEN"},
  {0.10, "GREEN"},
  {0.26, "GREEN"},
  {0.27, "YELLOW"},
  {0.30, "YELLOW"},
  {0.36, "YELLOW"},
  {0.37, "ORANGE"},
  {0.42, "ORANGE"},
  {0.47, "ORANGE"},
  {0.48, "RED"},
  {0.60, "RED"},
  {0.99, "RED"},
  {1.00, "BRIGHT RED"},
  {2.50, "BRIGHT RED"},
};

const int NUM_TESTS = sizeof(tests) / sizeof(tests[0]);
int currentTest = 0;
unsigned long lastSwitch = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  lavaLampBegin();

  Serial.println("=== LAVA LAMP COLOR TEST ===");
  Serial.println("Each price holds for 3 seconds.");
  Serial.println("Compare what you SEE on the strip to the expected color.");
  Serial.println();
}

void loop() {
  if (millis() - lastSwitch > 3000 || lastSwitch == 0) {
    if (currentTest >= NUM_TESTS) {
      Serial.println("=== ALL TESTS COMPLETE - restarting ===");
      Serial.println();
      currentTest = 0;
    }

    TestCase& t = tests[currentTest];
    LavaColor c = lavaColorForPrice(t.price);

    Serial.print("$");
    Serial.print(t.price, 2);
    Serial.print("  ->  R=");
    Serial.print(c.r);
    Serial.print(" G=");
    Serial.print(c.g);
    Serial.print(" B=");
    Serial.print(c.b);
    Serial.print(" brightness=");
    Serial.print(c.brightness);
    Serial.print("  [expected: ");
    Serial.print(t.expectedColor);
    Serial.println("]");

    currentTest++;
    lastSwitch = millis();
  }

  lavaLampUpdate(tests[currentTest - 1].price);
  delay(30);
}

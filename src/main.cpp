// ============================================================
// AmberOrb - Modular Amber Electric Display
// ============================================================
// Module selection is now in platformio.ini via build_flags
// (e.g. -D USE_LAVA_LAMP). No need to edit this file.
// ============================================================

#include <Arduino.h>
#include "WiFiS3.h"
#include "config.h"
#include "amber_api.h"

#ifdef USE_LAVA_LAMP
  #include "lava_lamp.h"
#endif

#ifdef USE_MATRIX_SUN
  #include "matrix_sun.h"
#endif

#ifdef USE_MATRIX_DIGITS
  #include "matrix_digits.h"
#endif

#ifdef USE_TRAFFIC_LEDS
  #include "traffic_leds.h"
#endif

unsigned long lastFetch = 0;
const unsigned long FETCH_INTERVAL = 300000;

// Used for cycling matrix displays when both sun and digits are on
#ifdef USE_MATRIX_SUN
#ifdef USE_MATRIX_DIGITS
  unsigned long lastDisplaySwitch = 0;
  int displayPhase = 0;  // 0=sun, 1=price, 2=percent
#endif
#endif

void setup() {
  Serial.begin(115200);
  delay(2000);

  // --- Initialise enabled modules ---
  #ifdef USE_LAVA_LAMP
    lavaLampBegin();
  #endif

  #ifdef USE_MATRIX_SUN
    matrixSunBegin();
  #endif

  // Only begin digits if sun isn't active (they share hardware)
  #ifdef USE_MATRIX_DIGITS
  #ifndef USE_MATRIX_SUN
    matrixDigitsBegin();
  #endif
  #endif

  #ifdef USE_TRAFFIC_LEDS
    trafficLedsBegin();
  #endif

  // --- Connect to WiFi ---
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  delay(2000);
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  // --- Startup animations ---
  #ifdef USE_MATRIX_SUN
    matrixSunStartupAnimation();
  #endif

  // --- Fetch initial data ---
  if (fetchAmberData()) {
    #ifdef USE_MATRIX_SUN
      matrixSunSetTarget(getRenewables());
    #endif
  }
  lastFetch = millis();
}

void loop() {
  // --- Fetch new data every 5 minutes ---
  if (millis() - lastFetch > FETCH_INTERVAL) {
    Serial.println("Fetching new data...");
    if (fetchAmberData()) {
      #ifdef USE_MATRIX_SUN
        matrixSunSetTarget(getRenewables());
      #endif
    }
    lastFetch = millis();
  }

  // --- Update each enabled module ---

  #ifdef USE_LAVA_LAMP
    lavaLampUpdate(getPrice());
  #endif

  #ifdef USE_TRAFFIC_LEDS
    trafficLedsUpdate(getRenewables());
  #endif

  // --- Matrix display logic ---
  // If both sun and digits are enabled, cycle between them
  #ifdef USE_MATRIX_SUN
  #ifdef USE_MATRIX_DIGITS
    if (millis() - lastDisplaySwitch > 4000) {
      displayPhase = (displayPhase + 1) % 3;
      lastDisplaySwitch = millis();
    }
    if (displayPhase == 0) {
      matrixSunUpdate();
    } else if (displayPhase == 1) {
      matrixDrawPrice(getPrice());
    } else {
      matrixDrawPercent(getRenewables());
    }
  #else
    // Just sun, no digits
    matrixSunUpdate();
  #endif
  #else
  #ifdef USE_MATRIX_DIGITS
    // Just digits, no sun — alternate price and percent
    static unsigned long lastDigitSwitch = 0;
    static bool showPrice = true;
    if (millis() - lastDigitSwitch > 4000) {
      showPrice = !showPrice;
      lastDigitSwitch = millis();
    }
    if (showPrice) {
      matrixDrawPrice(getPrice());
    } else {
      matrixDrawPercent(getRenewables());
    }
  #endif
  #endif

  delay(30);
}

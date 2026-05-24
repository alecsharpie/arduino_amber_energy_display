// AmberOrb - Amber Electric price display for Arduino Uno R4 WiFi
// Module selection via build_flags in platformio.ini (e.g. -D USE_LAVA_LAMP)

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
unsigned long lastWifiCheck = 0;

#ifdef DEV_MODE
  const unsigned long FETCH_INTERVAL = 60000;
#else
  const unsigned long FETCH_INTERVAL = 30000;
  static int lastFetchedSlot = -1;
#endif

#ifdef USE_MATRIX_SUN
#ifdef USE_MATRIX_DIGITS
  unsigned long lastDisplaySwitch = 0;
  int displayPhase = 0;
#endif
#endif

bool connectWiFi(unsigned long timeoutMs) {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(500);
  }
  return WiFi.status() == WL_CONNECTED;
}

void onFetchSuccess() {
  #ifdef USE_MATRIX_SUN
    matrixSunSetTarget(getRenewables());
  #endif
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  #ifdef USE_LAVA_LAMP
    lavaLampBegin();
  #endif

  #ifdef USE_MATRIX_SUN
    matrixSunBegin();
  #endif

  #ifdef USE_MATRIX_DIGITS
  #ifndef USE_MATRIX_SUN
    matrixDigitsBegin();
  #endif
  #endif

  #ifdef USE_TRAFFIC_LEDS
    trafficLedsBegin();
  #endif

  Serial.print("Connecting to WiFi");
  if (connectWiFi(30000)) {
    Serial.println("\nConnected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connect failed, will retry in loop");
  }

  #ifdef USE_MATRIX_SUN
    matrixSunStartupAnimation();
  #endif

  if (fetchAmberData()) {
    onFetchSuccess();
  }
  lastFetch = millis();
}

void loop() {
  // Throttled WiFi check (every 5s instead of every 30ms)
  if (millis() - lastWifiCheck > 5000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      if (connectWiFi(10000)) {
        Serial.println("WiFi reconnected!");
      } else {
        Serial.println("WiFi reconnect failed, will retry");
      }
    }
  }

  // Fetch price data
  bool shouldFetch = false;

  #ifdef DEV_MODE
    shouldFetch = (millis() - lastFetch > FETCH_INTERVAL);
  #else
    if (millis() - lastFetch > FETCH_INTERVAL) {
      unsigned long epoch = WiFi.getTime();
      if (epoch > 0) {
        int currentSlot = ((epoch / 60) % 60) / 30;  // 0 = :00-:29, 1 = :30-:59
        if (currentSlot != lastFetchedSlot) {
          shouldFetch = true;
        }
      }
      lastFetch = millis();
    }
  #endif

  if (shouldFetch) {
    Serial.println("Fetching price data...");
    if (fetchAmberData()) {
      onFetchSuccess();
      #ifndef DEV_MODE
        lastFetchedSlot = ((WiFi.getTime() / 60) % 60) / 30;
      #endif
    }
    #ifdef DEV_MODE
      lastFetch = millis();
    #endif
  }

  // Update display modules
  #ifdef USE_LAVA_LAMP
    lavaLampUpdate(getPrice());
  #endif

  #ifdef USE_TRAFFIC_LEDS
    trafficLedsUpdate(getRenewables());
  #endif

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
    matrixSunUpdate();
  #endif
  #else
  #ifdef USE_MATRIX_DIGITS
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

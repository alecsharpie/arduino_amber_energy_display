#include "amber_api.h"
#include "WiFiS3.h"
#include "config.h"
#include <ArduinoJson.h>

static float currentPrice = 0.0;
static float currentRenewables = 0.0;
static bool hasEverFetched = false;

float getPrice() { return currentPrice; }
float getRenewables() { return currentRenewables; }
bool hasPriceData() { return hasEverFetched; }

bool fetchAmberData() {
  WiFiSSLClient client;

  if (!client.connect("api.amber.com.au", 443)) {
    Serial.println("[API] Connection failed");
    return false;
  }

  String path = "/v1/sites/";
  path += AMBER_SITE_ID;
  path += "/prices/current";

  client.print("GET ");
  client.print(path);
  client.println(" HTTP/1.1");
  client.println("Host: api.amber.com.au");
  client.print("Authorization: Bearer ");
  client.println(AMBER_API_KEY);
  client.println("Connection: close");
  client.println();

  // Wait for response
  unsigned long timeout = millis();
  while (!client.available()) {
    if (millis() - timeout > 10000) {
      Serial.println("[API] Timeout waiting for response");
      client.stop();
      return false;
    }
  }

  // Skip HTTP headers
  String line;
  line.reserve(128);
  while (client.available()) {
    char c = client.read();
    if (c == '\n') {
      if (line.length() == 0 || line == "\r") break;
      line = "";
    } else {
      line += c;
    }
  }

  // Read full body before parsing (stream parsing from SSL is unreliable)
  String body;
  body.reserve(1536);
  timeout = millis();
  while (client.connected() || client.available()) {
    if (client.available()) {
      body += (char)client.read();
      timeout = millis();  // reset on progress
    }
    if (millis() - timeout > 5000) {
      Serial.println("[API] Timeout reading body");
      client.stop();
      return false;
    }
  }
  client.stop();

  if (body.length() == 0) {
    Serial.println("[API] Empty response body");
    return false;
  }

  // Parse JSON
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.print("[API] JSON error: ");
    Serial.println(err.c_str());
    return false;
  }

  // Find the "general" channel
  for (JsonObject obj : doc.as<JsonArray>()) {
    if (obj["channelType"] == "general") {
      float priceCents = obj["perKwh"];
      currentPrice = priceCents / 100.0;
      currentRenewables = obj["renewables"];
      hasEverFetched = true;

      Serial.print("[API] OK: ");
      Serial.print(priceCents, 4);
      Serial.print("c/kWh ($");
      Serial.print(currentPrice, 6);
      Serial.print(")  Renewables: ");
      Serial.print(currentRenewables, 1);
      Serial.println("%");
      return true;
    }
  }

  Serial.println("[API] No general channel in response");
  return false;
}

#include "amber_api.h"
#include "WiFiS3.h"
#include "config.h"
#include <ArduinoJson.h>

static float currentPrice = 0.0;
static float currentRenewables = 0.0;

float getPrice() { return currentPrice; }
float getRenewables() { return currentRenewables; }

bool fetchAmberData() {
  WiFiSSLClient client;

  if (!client.connect("api.amber.com.au", 443)) {
    Serial.println("API connection failed");
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

  unsigned long timeout = millis();
  while (!client.available()) {
    if (millis() - timeout > 10000) {
      Serial.println("API timeout");
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

  // Parse JSON directly from the stream (avoids storing full body in RAM)
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, client);
  client.stop();

  if (err) {
    Serial.print("JSON error: ");
    Serial.println(err.c_str());
    return false;
  }

  // Find the "general" channel in the response array
  for (JsonObject obj : doc.as<JsonArray>()) {
    if (obj["channelType"] == "general") {
      float priceCents = obj["perKwh"];
      currentPrice = priceCents / 100.0;
      currentRenewables = obj["renewables"];

      Serial.print("Price: ");
      Serial.print(priceCents);
      Serial.print("c/kWh  Renewables: ");
      Serial.print(currentRenewables);
      Serial.println("%");
      return true;
    }
  }

  Serial.println("No general channel found");
  return false;
}

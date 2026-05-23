#include "amber_api.h"
#include "WiFiS3.h"
#include "config.h"

static float currentPrice = 0.0;
static float currentRenewables = 0.0;

float getPrice() { return currentPrice; }
float getRenewables() { return currentRenewables; }

bool fetchAmberData() {
  WiFiSSLClient client;

  if (!client.connect("api.amber.com.au", 443)) {
    Serial.println("Connection to Amber API failed");
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
      Serial.println("API request timed out");
      client.stop();
      return false;
    }
  }

  bool headersEnded = false;
  String line = "";
  while (client.available() && !headersEnded) {
    char c = client.read();
    if (c == '\n') {
      if (line.length() == 0 || line == "\r") {
        headersEnded = true;
      }
      line = "";
    } else {
      line += c;
    }
  }

  String body = "";
  while (client.available()) {
    char c = client.read();
    body += c;
  }
  client.stop();

  Serial.println("API Response:");
  Serial.println(body);

  int generalIndex = body.indexOf("\"channelType\":\"general\"");
  if (generalIndex == -1) {
    Serial.println("Error: No general channel found");
    return false;
  }

  int perKwhIndex = body.indexOf("\"perKwh\":");
  if (perKwhIndex == -1) {
    Serial.println("Error: No perKwh field found");
    return false;
  }
  int valueStart = perKwhIndex + 9;
  int valueEnd = body.indexOf(",", valueStart);
  if (valueEnd == -1) valueEnd = body.indexOf("}", valueStart);
  float priceCents = body.substring(valueStart, valueEnd).toFloat();
  currentPrice = priceCents / 100.0;

  int renewIndex = body.indexOf("\"renewables\":");
  if (renewIndex == -1) {
    Serial.println("Error: No renewables field found");
    return false;
  }
  valueStart = renewIndex + 13;
  valueEnd = body.indexOf(",", valueStart);
  if (valueEnd == -1) valueEnd = body.indexOf("}", valueStart);
  currentRenewables = body.substring(valueStart, valueEnd).toFloat();

  Serial.print("Price: ");
  Serial.print(priceCents);
  Serial.print(" c/kWh ($");
  Serial.print(currentPrice, 2);
  Serial.println(")");
  Serial.print("Renewables: ");
  Serial.print(currentRenewables);
  Serial.println("%");

  return true;
}
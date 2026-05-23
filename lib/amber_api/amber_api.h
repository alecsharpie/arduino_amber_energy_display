#ifndef AMBER_API_H
#define AMBER_API_H

// Call once in setup() after WiFi connects
// Call periodically in loop() to refresh data
bool fetchAmberData();

// Access the latest values after a successful fetch
float getPrice();        // in $/kWh (e.g. 0.28)
float getRenewables();   // percentage (e.g. 45.5)

#endif
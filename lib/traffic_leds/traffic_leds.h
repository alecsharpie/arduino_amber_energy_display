#ifndef TRAFFIC_LEDS_H
#define TRAFFIC_LEDS_H

// Call once in setup()
void trafficLedsBegin();

// Call in loop() with renewables percentage
// Green >= 50%, Yellow 20-50%, Red < 20%
void trafficLedsUpdate(float renewablesPercent);

#endif
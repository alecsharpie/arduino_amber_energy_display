#ifndef AMBER_API_H
#define AMBER_API_H

bool fetchAmberData();
float getPrice();        // $/kWh (e.g. 0.28)
float getRenewables();   // percentage (e.g. 45.5)
bool hasPriceData();     // true after first successful fetch

#endif

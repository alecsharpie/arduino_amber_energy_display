#ifndef MATRIX_SUN_H
#define MATRIX_SUN_H

#include "Arduino_LED_Matrix.h"

// Call once in setup()
void matrixSunBegin();

// Call in loop() - animates sun toward target stage
// Returns the current stage (0-7)
int matrixSunUpdate();

// Set the target stage based on renewables percentage (0-100)
void matrixSunSetTarget(float renewablesPercent);

// Play the startup bloom animation
void matrixSunStartupAnimation();

#endif
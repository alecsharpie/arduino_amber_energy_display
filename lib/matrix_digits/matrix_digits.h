#ifndef MATRIX_DIGITS_H
#define MATRIX_DIGITS_H

#include "Arduino_LED_Matrix.h"

// Call once in setup() - only needed if not using matrix_sun
// (they share the same hardware, so only begin once)
void matrixDigitsBegin();

// Display a price in X.XX format on the matrix
void matrixDrawPrice(float price);

// Display a percentage in XX% format on the matrix
void matrixDrawPercent(float percent);

#endif
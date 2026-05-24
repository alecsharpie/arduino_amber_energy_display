#include "matrix_digits.h"

static ArduinoLEDMatrix matrix;

static const byte digits[][7][3] = {
  {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}},
  {{0,1,0},{1,1,0},{0,1,0},{0,1,0},{0,1,0},{0,1,0},{1,1,1}},
  {{1,1,1},{0,0,1},{0,0,1},{1,1,1},{1,0,0},{1,0,0},{1,1,1}},
  {{1,1,1},{0,0,1},{0,0,1},{1,1,1},{0,0,1},{0,0,1},{1,1,1}},
  {{1,0,1},{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1},{0,0,1}},
  {{1,1,1},{1,0,0},{1,0,0},{1,1,1},{0,0,1},{0,0,1},{1,1,1}},
  {{1,1,1},{1,0,0},{1,0,0},{1,1,1},{1,0,1},{1,0,1},{1,1,1}},
  {{1,1,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1}},
  {{1,1,1},{1,0,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1},{1,1,1}},
  {{1,1,1},{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1},{1,1,1}}
};

static void drawDigit(byte frame[8][12], int digit, int colOffset) {
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 3; col++) {
      frame[row][col + colOffset] = digits[digit][row][col];
    }
  }
}

void matrixDigitsBegin() {
  matrix.begin();
}

void matrixDrawPrice(float price) {
  static float lastPrice = -1.0;
  if (price == lastPrice) return;
  lastPrice = price;

  byte frame[8][12] = { 0 };
  price = round(price * 100.0) / 100.0;
  if (price > 9.99) price = 9.99;
  if (price < 0.0) price = 0.0;

  int d1 = (int)price;
  int d2 = (int)(price * 10) % 10;
  int d3 = (int)(price * 100) % 10;

  drawDigit(frame, d1, 0);
  frame[7][3] = 1;
  drawDigit(frame, d2, 4);
  drawDigit(frame, d3, 8);
  matrix.renderBitmap(frame, 8, 12);
}

void matrixDrawPercent(float percent) {
  static float lastPercent = -1.0;
  if (percent == lastPercent) return;
  lastPercent = percent;

  byte frame[8][12] = { 0 };
  int pct = constrain((int)round(percent), 0, 99);
  int d1 = pct / 10;
  int d2 = pct % 10;

  drawDigit(frame, d1, 1);
  drawDigit(frame, d2, 5);

  frame[0][9] = 1; frame[0][10] = 1;
  frame[1][9] = 1; frame[1][10] = 1;
  frame[2][11] = 1;
  frame[3][10] = 1;
  frame[4][9] = 1;
  frame[5][10] = 1; frame[5][11] = 1;
  frame[6][10] = 1; frame[6][11] = 1;

  matrix.renderBitmap(frame, 8, 12);
}
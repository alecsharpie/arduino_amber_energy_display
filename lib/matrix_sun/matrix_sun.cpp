#include "matrix_sun.h"

static ArduinoLEDMatrix matrix;
static int currentStage = 0;
static int targetStage = 0;

static const byte sunFrames[][7][7] = {
  {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},
  {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,1,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},
  {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,1,0,0,0},{0,0,1,1,1,0,0},{0,0,0,1,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},
  {{0,0,0,0,0,0,0},{0,0,0,1,0,0,0},{0,0,1,1,1,0,0},{0,1,1,1,1,1,0},{0,0,1,1,1,0,0},{0,0,0,1,0,0,0},{0,0,0,0,0,0,0}},
  {{0,0,0,1,0,0,0},{0,1,0,1,0,1,0},{0,0,1,1,1,0,0},{1,1,1,1,1,1,1},{0,0,1,1,1,0,0},{0,1,0,1,0,1,0},{0,0,0,1,0,0,0}},
  {{1,0,0,1,0,0,1},{0,1,0,1,0,1,0},{0,0,1,1,1,0,0},{1,1,1,1,1,1,1},{0,0,1,1,1,0,0},{0,1,0,1,0,1,0},{1,0,0,1,0,0,1}},
  {{1,0,1,1,1,0,1},{0,1,1,1,1,1,0},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{0,1,1,1,1,1,0},{1,0,1,1,1,0,1}},
  {{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1},{1,1,1,1,1,1,1}}
};

static void drawSun(int stage) {
  byte frame[8][12] = { 0 };
  for (int row = 0; row < 7; row++) {
    for (int col = 0; col < 7; col++) {
      frame[row][col + 3] = sunFrames[stage][row][col];
    }
  }
  matrix.renderBitmap(frame, 8, 12);
}

void matrixSunBegin() {
  matrix.begin();
}

int matrixSunUpdate() {
  if (currentStage < targetStage) currentStage++;
  else if (currentStage > targetStage) currentStage--;
  drawSun(currentStage);
  return currentStage;
}

void matrixSunSetTarget(float renewablesPercent) {
  targetStage = map(constrain((int)renewablesPercent, 0, 100), 0, 100, 0, 7);
}

void matrixSunStartupAnimation() {
  for (int i = 0; i <= 7; i++) { drawSun(i); delay(150); }
  for (int i = 7; i >= 0; i--) { drawSun(i); delay(150); }
}
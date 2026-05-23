// ============================================================
// Unit tests for lava lamp color logic
// ============================================================
// Run with: pio test -e native
//
// Uses Unity — PlatformIO's built-in test framework.
// Each TEST_ASSERT checks one condition and reports PASS/FAIL.
// ============================================================

#include <unity.h>
#include "lava_colors.h"  // shared with main firmware — single source of truth

// Helper to assert a color matches expected RGB, brightness, and speed
static void assertColor(float price, int r, int g, int b, int brightness, float speed) {
  LavaColor c = lavaColorForPrice(price);
  TEST_ASSERT_EQUAL_INT(r, c.r);
  TEST_ASSERT_EQUAL_INT(g, c.g);
  TEST_ASSERT_EQUAL_INT(b, c.b);
  TEST_ASSERT_EQUAL_INT(brightness, c.brightness);
  TEST_ASSERT_FLOAT_WITHIN(0.01, speed, c.speed);
}

// --- GREEN band: < 27c ---

void test_green_at_zero(void) {
  assertColor(0.00, 0, 255, 0, 18, 1.0);
}

void test_green_at_10c(void) {
  assertColor(0.10, 0, 255, 0, 18, 1.0);
}

void test_green_at_26c(void) {
  assertColor(0.26, 0, 255, 0, 18, 1.0);
}

// --- YELLOW band: 27c–36c ---

void test_yellow_at_27c(void) {
  assertColor(0.27, 255, 150, 0, 30, 1.0);
}

void test_yellow_at_30c(void) {
  assertColor(0.30, 255, 150, 0, 30, 1.0);
}

void test_yellow_at_36c(void) {
  assertColor(0.36, 255, 150, 0, 30, 1.0);
}

// --- ORANGE band: 37c–47c ---

void test_orange_at_37c(void) {
  assertColor(0.37, 255, 50, 0, 30, 1.0);
}

void test_orange_at_42c(void) {
  assertColor(0.42, 255, 50, 0, 30, 1.0);
}

void test_orange_at_47c(void) {
  assertColor(0.47, 255, 50, 0, 30, 1.0);
}

// --- RED band: 48c–99c ---

void test_red_at_48c(void) {
  assertColor(0.48, 255, 0, 0, 30, 1.0);
}

void test_red_at_60c(void) {
  assertColor(0.60, 255, 0, 0, 30, 1.0);
}

void test_red_at_99c(void) {
  assertColor(0.99, 255, 0, 0, 30, 1.0);
}

// --- FAST RED band: >= $1 (same colour, fast animation) ---

void test_bright_red_at_1_dollar(void) {
  assertColor(1.00, 255, 0, 0, 30, 8.0);
}

void test_bright_red_at_2_50(void) {
  assertColor(2.50, 255, 0, 0, 30, 8.0);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_green_at_zero);
  RUN_TEST(test_green_at_10c);
  RUN_TEST(test_green_at_26c);

  RUN_TEST(test_yellow_at_27c);
  RUN_TEST(test_yellow_at_30c);
  RUN_TEST(test_yellow_at_36c);

  RUN_TEST(test_orange_at_37c);
  RUN_TEST(test_orange_at_42c);
  RUN_TEST(test_orange_at_47c);

  RUN_TEST(test_red_at_48c);
  RUN_TEST(test_red_at_60c);
  RUN_TEST(test_red_at_99c);

  RUN_TEST(test_bright_red_at_1_dollar);
  RUN_TEST(test_bright_red_at_2_50);

  return UNITY_END();
}

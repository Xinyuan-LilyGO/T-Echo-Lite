/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2025-09-12 16:42:57
 * @LastEditTime: 2026-04-25 09:54:40
 * @License: GPL 3.0
 */
#include "Adafruit_NeoPixel.h"
#include "t_echo_lite_config.h"

#define NUM_LEDS 10

Adafruit_NeoPixel g_led(NUM_LEDS, EXT_1x4P_1_IO_0_23, NEO_GRB + NEO_KHZ800);

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536;
       firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    g_led.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    g_led.show();  // Update strip with new contents
    delay(wait);   // Pause for a moment
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Ciallo");

  // 3.3V Power ON
  pinMode(RT9080_EN, OUTPUT);
  digitalWrite(RT9080_EN, HIGH);

  g_led.begin();
  g_led.show();
  g_led.setBrightness(100);

  for (size_t i = 0; i < NUM_LEDS; i++) {
    g_led.setPixelColor(i, g_led.Color(255, 0, 0));
  }
  g_led.show();
  delay(500);

  for (size_t i = 0; i < NUM_LEDS; i++) {
    g_led.setPixelColor(i, g_led.Color(0, 255, 0));
  }
  g_led.show();
  delay(500);

  for (size_t i = 0; i < NUM_LEDS; i++) {
    g_led.setPixelColor(i, g_led.Color(0, 0, 255));
  }
  g_led.show();
  delay(500);

  for (size_t i = 0; i < NUM_LEDS; i++) {
    g_led.setPixelColor(i, g_led.Color(255, 255, 255));
  }
  g_led.show();
  delay(500);

  for (size_t i = 0; i < NUM_LEDS; i++) {
    g_led.setPixelColor(i, g_led.Color(0, 0, 0));
  }
  g_led.show();
  delay(500);
}

void loop() {
  rainbow(100);  // Flowing rainbow cycle along the whole strip
}
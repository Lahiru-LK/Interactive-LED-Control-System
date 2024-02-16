#include <AccelStepper.h>
#include <MultiStepper.h>
#include <FastLED.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

namespace LEDFunctions {
  const int LED_COUNT = 100;
  CRGB leds[LED_COUNT];

  bool lightsOn = true;
  int lightsIntensity = 100;
  int lightsSpeed = 50;
  int lightsPattern = 0;  // 0: Color Wipe, 1: Twinkle, etc.

  void twinkleRandom(int count) {
    for (int i = 0; i < count; ++i) {
      int index = random(LED_COUNT);
      leds[index] = CRGB::White;
      FastLED.show();
      delay(50);
      leds[index] = CRGB::Black;
    }
  }

void setup() {
  
}

void loop() {
  
}
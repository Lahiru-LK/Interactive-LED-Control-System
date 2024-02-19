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

  void colorWipe(CRGB color, int wait) {
    for (int i = 0; i < LEDFunctions::LED_COUNT; ++i) {
      leds[i] = color;
      FastLED.show();
      delay(wait);
    }
  }

  void changeLightsPattern(int pattern) {
    lightsPattern = pattern;
  }

  void toggleLights() {
    lightsOn = !lightsOn;
  }
}

#define ENCODER_DT 18
#define ENCODER_CLK 19
#define ENCODER_SW 20

#define LED_PIN       6
#define COLOR_ORDER   GRB
#define CHIPSET       WS2811
#define BRIGHTNESS    200
#define FRAMES_PER_SECOND 30
#define COOLING       60
#define SPARKING      160

CRGB leds[LEDFunctions::LED_COUNT];

AccelStepper stepper1(AccelStepper::FULL4WIRE, 2, 4, 3, 5);
AccelStepper stepper2(AccelStepper::FULL4WIRE, 8, 10, 9, 11);
AccelStepper stepper3(AccelStepper::FULL4WIRE, 12, 14, 13, 15);

MultiStepper steppers;

LiquidCrystal_I2C lcd(0x27, 20, 4);
Encoder encoder(ENCODER_DT, ENCODER_CLK);

int selectedStepper = 1;
int encoderPosition;
int lastEncoderPosition;

int currentPage = 0;
int selectedMenuItem = 0;
const int NUM_STEPPERS = 3;
const int NUM_LED_GROUPS = 4;
const int HOME_PAGE = 0;
const int STEPPERS_PAGE = 1;
const int LIGHTS_PAGE = 2;
const int SETTINGS_PAGE = 3;

int currentSpeed = 0;
bool rotationDirection = true;
int selectedLightGroup = 0;
bool stepperState[NUM_STEPPERS] = {false};

void setup() {
  Serial.begin(9600);

  stepper1.setMaxSpeed(100);
  stepper2.setMaxSpeed(100);
  stepper3.setMaxSpeed(100);

  steppers.addStepper(stepper1);
  steppers.addStepper(stepper2);
  steppers.addStepper(stepper3);

  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(LEDFunctions::leds, LEDFunctions::LED_COUNT);

  FastLED.setBrightness(BRIGHTNESS);

  lcd.init();
  lcd.backlight();

  printAboutPage();
}

void loop() {
  handleMenuNavigation();
  
  if (buttonPressed()) {
    handleMenuSelection();
  }

  updateLCD();
  updateSteppers();
  updateLEDs();
  handleEncoderInput();
  updateLCD();

  // Keep motors and lights spinning unless turned off in the settings
  if (stepperState[selectedStepper - 1] && LEDFunctions::lightsOn) {
    updateStepperRotation(selectedStepper);
    updateLEDs();
  }

  delay(100);
}




void handleMenuNavigation() {
  encoderPosition = encoder.read();

  if (encoderPosition != lastEncoderPosition) {
    if (encoderPosition > lastEncoderPosition) {
      incrementMenu();
    } else {
      decrementMenu();
    }

    lastEncoderPosition = encoderPosition;
  }
}


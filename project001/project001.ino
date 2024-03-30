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


void incrementMenu() {
  switch (currentPage) {
    case HOME_PAGE:
      selectedMenuItem = (selectedMenuItem + 1) % 3;
      break;

    case STEPPERS_PAGE:
      selectedMenuItem = (selectedMenuItem + 1) % 2;
      break;

    case LIGHTS_PAGE:
    case SETTINGS_PAGE:
      selectedMenuItem = (selectedMenuItem + 1) % 4;
      break;
  }
}

void decrementMenu() {
  switch (currentPage) {
    case HOME_PAGE:
      selectedMenuItem = (selectedMenuItem + 2) % 3;
      break;

    case STEPPERS_PAGE:
      selectedMenuItem = (selectedMenuItem + 1) % 2;
      break;

    case LIGHTS_PAGE:
    case SETTINGS_PAGE:
      selectedMenuItem = (selectedMenuItem + 3) % 4;
      break;
  }
}


bool buttonPressed() {
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  unsigned long debounceDelay = 50;

  int buttonState = digitalRead(ENCODER_SW);

  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW) {
      return true;
    }
  }

  lastButtonState = buttonState;

  return false;
}

void handleMenuSelection() {
  switch (currentPage) {
    case HOME_PAGE:
      handleHomeMenuSelection();
      break;

    case STEPPERS_PAGE:
      handleSteppersMenuSelection();
      break;

    case LIGHTS_PAGE:
      handleLightsMenuSelection();  // Add this line
      break;

    case SETTINGS_PAGE:
      break;
  }
}


void handleHomeMenuSelection() {
  switch (selectedMenuItem) {
    case 0:
      updateAndDisplayStatus();
      break;

    case 1:
      currentPage = STEPPERS_PAGE;
      break;

    case 2:
      printAboutPage();
      break;
  }
}

void updateAndDisplayStatus() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Status");
  lcd.setCursor(0, 2);
  lcd.print("Selected Stepper: ");
  lcd.print(selectedStepper);
}

void handleSteppersMenuSelection() {
  switch (selectedMenuItem) {
    case 0:
      updateStepperOnOff(selectedStepper);
      break;

    case 1:
      updateStepperSpeed(selectedStepper);
      break;

    case 2:
      updateStepperRotation(selectedStepper);
      break;
  }
}

void updateStepperOnOff(int stepperNumber) {
  stepperState[stepperNumber - 1] = !stepperState[stepperNumber - 1];
  updateLCD();
}

void updateStepperSpeed(int stepperNumber) {
  int newSpeed = promptUserForNewSpeed();

  switch (stepperNumber) {
    case 1:
      stepper1.setMaxSpeed(newSpeed);
      break;
    case 2:
      stepper2.setMaxSpeed(newSpeed);
      break;
    case 3:
      stepper3.setMaxSpeed(newSpeed);
      break;
  }

  updateLCD();
}

void updateStepperRotation(int stepperNumber) {
  bool isClockwise = promptUserForRotationDirection();
  int speedMultiplier = isClockwise ? 1 : -1;

  switch (stepperNumber) {
    case 1:
      stepper1.setSpeed(currentSpeed * speedMultiplier);
      break;
    case 2:
      stepper2.setSpeed(currentSpeed * speedMultiplier);
      break;
    case 3:
      stepper3.setSpeed(currentSpeed * speedMultiplier);
      break;
  }

  updateLCD();
}

int promptUserForNewSpeed() {
  Serial.println("Enter new speed for the stepper:");
  while (!Serial.available()) {
  }
  int newSpeed = Serial.parseInt();
  Serial.println("Speed updated!");

  return newSpeed;
}

int promptUserForRotationDirection() {
  Serial.println("Enter rotation direction (1 for CW, 0 for CCW):");
  while (!Serial.available()) {
  }
  int isClockwise = Serial.read() - '0';
  Serial.println("Rotation direction updated!");

  return isClockwise;
}

void updateSteppers() {
  switch (selectedStepper) {
    case 1:
      updateIndividualStepper(stepper1);
      break;
    case 2:
      updateIndividualStepper(stepper2);
      break;
    case 3:
      updateIndividualStepper(stepper3);
      break;
  }
}

void updateIndividualStepper(AccelStepper& stepper) {
  stepper.setMaxSpeed(currentSpeed);
  stepper.setSpeed(rotationDirection ? stepper.maxSpeed() : -stepper.maxSpeed());

  long newPosition = calculateNewTargetPosition();
  steppers.moveTo(newPosition);

  if (stepper.isRunning()) {
    stepperState[selectedStepper - 1] = true;
  } else {
    stepperState[selectedStepper - 1] = false;
  }
}

long calculateNewTargetPosition() {
  static long targetPosition = 0;
  const long incrementAmount = 100;
  targetPosition += incrementAmount;

  return targetPosition;
}

void updateLEDs() {
  if (LEDFunctions::lightsOn) {
    switch (LEDFunctions::lightsPattern) {
      case 0:  // Color Wipe
        fill_rainbow(LEDFunctions::leds, LEDFunctions::LED_COUNT, 0, 255 / LEDFunctions::LED_COUNT);
        LEDFunctions::colorWipe(CRGB::Blue, LEDFunctions::lightsSpeed);
        break;

      case 1:  // Twinkle
        LEDFunctions::twinkleRandom(20);
        break;

      // Add more cases for different light patterns as needed

      default:
        break;
    }
  } else {
    fill_solid(LEDFunctions::leds, LEDFunctions::LED_COUNT, CRGB::Black);
  }

  FastLED.show();
}

void handleEncoderInput() {
  if (encoderPosition > lastEncoderPosition) {
    selectedStepper = (selectedStepper % NUM_STEPPERS) + 1;
  } else {
    selectedStepper = (selectedStepper + NUM_STEPPERS - 1) % NUM_STEPPERS + 1;
  }
}

void updateLCD() {
  lcd.clear();

  switch (currentPage) {
    case HOME_PAGE:
      lcd.print("Home Page");
      lcd.setCursor(0, 1);
      lcd.print("MenuItem: ");
      lcd.print(selectedMenuItem);
      break;

    case STEPPERS_PAGE:
      lcd.print("Steppers Page");
      lcd.setCursor(0, 1);
      lcd.print("Stepper: ");
      lcd.print(selectedStepper);
      lcd.setCursor(0, 2);
      lcd.print("Speed: ");
      lcd.print(currentSpeed);
      lcd.setCursor(0, 3);
      lcd.print("Direction: ");
      lcd.print(rotationDirection ? "CW" : "CCW");
      lcd.setCursor(0, 4);
      lcd.print("State: ");
      lcd.print(stepperState[selectedStepper - 1] ? "ON" : "OFF");
      break;

    case LIGHTS_PAGE:
      lcd.print("Lights Page");
      lcd.setCursor(0, 1);
      lcd.print("1. Lights On/Off");
      lcd.setCursor(0, 2);
      lcd.print("2. Lights Intensity");
      lcd.setCursor(0, 3);
      lcd.print("3. Lights Speed");
      lcd.setCursor(0, 4);
      lcd.print("4. Lights Pattern");
      break;

    case SETTINGS_PAGE:
      printSettingsPage();
      break;
  }
}

void printAboutPage() {
  lcd.clear();
  lcd.print("Device Name");
  lcd.setCursor(0, 1);
  lcd.print("Version X.X");
  delay(3000);
}

void handleLightsMenuSelection() {
  switch (selectedMenuItem) {

    case 1:
      updateLightsOnOff();
      break;

    case 2:
      updateLightsIntensity();
      break;

    case 3:
      updateLightsSpeed();
      break;

    case 4:
      updateLightsPattern();
      break;
  }
}

void updateLightsOnOff() {
  LEDFunctions::toggleLights();
  updateLCD();
}

void updateLightsIntensity() {
  Serial.println("Enter new lights intensity (0-255):");
  while (!Serial.available()) {
  }
  int newIntensity = constrain(Serial.parseInt(), 0, 255);
  Serial.println("Lights intensity updated!");

  LEDFunctions::lightsIntensity = newIntensity;
  FastLED.setBrightness(newIntensity);
  updateLCD();
}

void updateLightsSpeed() {
  Serial.println("Enter new lights speed:");
  while (!Serial.available()) {
  }
  int newSpeed = Serial.parseInt();
  Serial.println("Lights speed updated!");

  LEDFunctions::lightsSpeed = newSpeed;
  updateLCD();
}

void updateLightsPattern() {
  Serial.println("Enter new lights pattern (0 for Color Wipe, 1 for Twinkle, etc.):");
  while (!Serial.available()) {
  }
  int newPattern = Serial.parseInt();
  Serial.println("Lights pattern updated!");

  LEDFunctions::changeLightsPattern(newPattern);
  updateLCD();
}

void printSettingsPage() {
  lcd.print("Settings Page");
  lcd.setCursor(0, 1);
  lcd.print("1. Stepper Settings");
  lcd.setCursor(0, 2);
  lcd.print("2. Lights Settings");
  lcd.setCursor(0, 3);
  lcd.print("3. Back");
}



/*
  FaceCycle — cycle Sesame face bitmaps on a 128 x 64 SSD1306 display.

  SETUP
  1. Put the original face-bitmaps.h next to this FaceCycle.ino file.
     https://github.com/dorianborian/sesame-studio/blob/main/assets/firmware/face-bitmaps.h
  2. In Arduino Library Manager, install:
       Adafruit SSD1306
       Adafruit GFX Library
     Accept their required dependencies.
  3. Select your board and upload.

  Default wiring for a CLASSIC ESP32 development board:
    Display GND -> board GND (ground)
    Display VCC -> board 3V3 (3.3-volt supply)
    Display SDA -> board GPIO 21 (serial data)
    Display SCL -> board GPIO 22 (serial clock)
  GPIO means general-purpose input/output.
  I2C means Inter-Integrated Circuit; OLED means organic light-emitting diode.
  Other ESP32 variants may require different pins: edit DATA_PIN/CLOCK_PIN.
  Other Arduino boards use their default Wire pins; available memory must
  accommodate the face collection plus a 1,024-byte display buffer.

  Shows the ROOT bitmap of each FACE_LIST entry in header order, repeatedly.
  Numbered animation frames are not played. Edit FACE_HOLD_MS for speed.
  Detects the display at address 0x3C or 0x3D. Serial diagnostics: 115200 baud.

  Compatibility: written against Sesame's documented FACE_LIST and
  epd_bitmap_<name> conventions. The exact linked header was inaccessible
  during preparation; compilation against that file has not been verified.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "face-bitmaps.h"

#ifndef FACE_LIST
#error "face-bitmaps.h must define Sesame's object-like FACE_LIST macro."
#endif

constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int DATA_PIN = 21;
constexpr int CLOCK_PIN = 22;
constexpr unsigned long FACE_HOLD_MS = 2000;
constexpr uint8_t ROTATION = 0;  // Set to 2 to rotate the display 180 degrees.

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Generate one pointer per face using the header's own registration list.
#define X(name) epd_bitmap_##name,
const unsigned char* const faceBitmaps[] = {
  FACE_LIST
};
#undef X

constexpr size_t FACE_COUNT = sizeof(faceBitmaps) / sizeof(faceBitmaps[0]);
static_assert(FACE_COUNT > 0, "The face list is empty.");
static_assert(FACE_HOLD_MS > 0, "The display interval must be positive.");

size_t faceIndex = 0;
unsigned long lastChange = 0;

void stopWithError(const __FlashStringHelper* message) {
  Serial.println(message);
  while (true) {
    delay(1000);
  }
}

uint8_t findDisplayAddress() {
  const uint8_t candidates[] = {0x3C, 0x3D};
  for (uint8_t address : candidates) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      return address;
    }
  }
  return 0;
}

void showFace() {
  display.clearDisplay();
  if (faceBitmaps[faceIndex] != nullptr) {
    display.drawBitmap(0, 0, faceBitmaps[faceIndex],
                       SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  }
  display.display();
  Serial.print(F("Face "));
  Serial.print(static_cast<unsigned long>(faceIndex + 1));
  Serial.print(F(" / "));
  Serial.println(static_cast<unsigned long>(FACE_COUNT));
}

void setup() {
  Serial.begin(115200);

#if defined(ARDUINO_ARCH_ESP32)
  Wire.begin(DATA_PIN, CLOCK_PIN);
#else
  Wire.begin();
#endif

  const uint8_t address = findDisplayAddress();
  if (address == 0) {
    stopWithError(F("No display at 0x3C or 0x3D. Check power and wiring."));
  }

  // Wire is already initialized; the fourth argument prevents reinitialization.
  if (!display.begin(SSD1306_SWITCHCAPVCC, address, true, false)) {
    stopWithError(F("Could not allocate the display buffer."));
  }

  display.setRotation(ROTATION);
  display.invertDisplay(false);
  Serial.print(F("Display address: 0x"));
  Serial.println(address, HEX);
  showFace();
  lastChange = millis();
}

void loop() {
  const unsigned long now = millis();
  // Unsigned subtraction also handles the millis() counter wrapping around.
  if (now - lastChange >= FACE_HOLD_MS) {
    faceIndex = (faceIndex + 1) % FACE_COUNT;
    showFace();
    lastChange = millis();
  }
  delay(1);
}

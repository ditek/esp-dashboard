#pragma once

#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C // See datasheet for Address
#define OLED_RESET -1       // The display doesn't have a dedicated reset pin

extern Adafruit_SSD1306 display;

void setupDisplay();

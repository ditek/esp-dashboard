#pragma once

#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "types.hpp"

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C // See datasheet for Address
#define OLED_RESET -1       // The display doesn't have a dedicated reset pin
#define NUM_DISPLAYS 6      // The number of displays connected to the I2C mux

extern Adafruit_SSD1306 display;

void setupDisplay();

// Clear and set cursor to (0,0)
void resetDisplay();

// Select the display with index i
void selectDisplay(uint8_t i);

// Dispaly each element in the vector in a display
void displayDataVector(std::vector<dataItem> dataVector, bool withSuffix);
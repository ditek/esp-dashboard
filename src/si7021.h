#pragma once

#include "Adafruit_Si7021.h"

extern Adafruit_Si7021 si7021;

void setupSi7021();
float getHumidity();
float getTemperature();
void enableSi7021Heater(bool enable);
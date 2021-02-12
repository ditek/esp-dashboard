#pragma once

#ifndef _CCS811_UTILS_H_
#define _CCS811_UTILS_H_

#include <Arduino.h>
#include "ccs811.h"  // CCS811 library

// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
extern CCS811 ccs811;

void setupSensor();
void runSensor();

#endif
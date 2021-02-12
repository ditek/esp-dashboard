#pragma once

#include <Arduino.h>
#include "ccs811.h"  // CCS811 library

// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
extern CCS811 ccs811;

typedef struct sensorReading {
    uint16_t eco2, etvoc;
    bool valid;

    sensorReading(uint16_t co2, uint16_t voc) : eco2(co2), etvoc(voc), valid(true) {}
    sensorReading(uint16_t co2, uint16_t voc, bool valid) : eco2(co2), etvoc(voc), valid(valid) {}
    sensorReading(bool valid) : valid(valid) {}
} sensorReading;

void setupSensor();
sensorReading getSensorReading();

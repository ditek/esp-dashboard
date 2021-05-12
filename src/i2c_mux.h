/**
 * TCA9548 I2C multiplexer
 */

#pragma once

#include "Wire.h"

#define TCAADDR 0x70

void i2c_select(uint8_t i);
void scan();

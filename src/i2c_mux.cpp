
#include "i2c_mux.h"
#include "Arduino.h"

void i2c_select(uint8_t i) {
    if (i > 7) return;

    Wire.beginTransmission(TCAADDR);
    Wire.write(1 << i);
    Wire.endTransmission();
}

void scan() {
    Serial.println("Scanning...");

    int nDevices = 0;

    for (uint8_t t = 0; t < 8; t++) {
        i2c_select(t);
        Serial.print("TCA Port #"); Serial.println(t);

        for (uint8_t addr = 1; addr < 127; addr++) {
            if (addr == TCAADDR) continue;

            Wire.beginTransmission(addr);
            if (!Wire.endTransmission()) {
                Serial.print("Found I2C 0x");  Serial.println(addr, HEX);
                nDevices++;
            }
        }
    }
    Serial.println("No I2C devices found\n");
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
}

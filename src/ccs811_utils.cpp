#include <Wire.h>
#include "ccs811_utils.h"

CCS811 ccs811(D3); // nWAKE on D3
uint8_t errCnt;

void setupSensor() {
    // Enable I2C
    Wire.begin();

    // Enable CCS811
    ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
    bool ok = ccs811.begin();
    if (!ok) Serial.println("setup: CCS811 begin FAILED");

    // Start measuring
    ok = ccs811.start(CCS811_MODE_1SEC);
    if (!ok) Serial.println("setup: CCS811 start FAILED");
}

sensorReading getSensorReading() {
    uint16_t eco2, etvoc, errstat, raw;
    ccs811.read(&eco2, &etvoc, &errstat, &raw);

    // Print measurement results based on status
    if (errstat == CCS811_ERRSTAT_OK) {
        return sensorReading(eco2, etvoc);
    }
    else if (errstat == CCS811_ERRSTAT_OK_NODATA) {
        Serial.println("CCS811: waiting for (new) data");
    }
    else if (errstat & CCS811_ERRSTAT_I2CFAIL) {
        Serial.println("CCS811: I2C error");
    }
    else {
        Serial.print("CCS811: errstat="); Serial.print(errstat, HEX);
        Serial.print("="); Serial.println(ccs811.errstat_str(errstat));
        errCnt++;
        if (errCnt > 10) {
            setupSensor();
            errCnt = 0;
        }
    }
    return sensorReading(false);
}
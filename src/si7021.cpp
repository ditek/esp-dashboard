#include "si7021.h"

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_Si7021 si7021 = Adafruit_Si7021();

void setupSi7021() {
    if (!si7021.begin()) {
        Serial.println("Did not find Si7021 sensor!");
    }
}

float getHumidity() {
    return si7021.readHumidity();
}

float getTemperature() {
    return si7021.readTemperature();
}

// If humidity is >80%,condensation may occur on the sensor.
// The on-chip heater can be activated to dry the sensor and restore operation.
// An ~1.8 degC temperature increase can be noted when heater is enabled
void enableSi7021Heater( bool enable){
    si7021.heater(enable);
}
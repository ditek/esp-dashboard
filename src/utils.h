#pragma once

#include <Arduino.h>

String floatToString(float f) {
    char buf[5];
    auto val_int = (int)f;
    auto val_fra = (int)((f - (float)val_int) * 10);
    snprintf(buf, sizeof(buf), "%d.%d", val_int, val_fra);
    return buf;
}
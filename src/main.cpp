#include <Arduino.h>
#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Create from `config_template.h`
#include "config.h"
#include "types.hpp"
#include "regex_matcher.h"
#include "display.h"
#include "ccs811_utils.h"
#include "si7021.h"
#include "utils.h"

using std::vector;

// Text looks like "<a href="#">Fajr</a><span>06:08</span></li>"
const char* regExpression = ">([a-zA-Z]+)</[span]{1,4}><span>([0-9]{2}:[0-9]{2})</span>";
const String url = "http://islam.no/prayer/get/85";
// const String url = "http://jigsaw.w3.org/HTTP/connection.html";

ESP8266WiFiMulti WiFiMulti;

#define NUM_PRAYER_TIMES 6
#define NUM_SENSOR_DATA 4
#define CO2_INDEX 0
#define VOC_INDEX 1
#define TEMP_INDEX 2
#define HUMIDITY_INDEX 3

// vector<dataItem> dataVector(NUM_PRAYER_TIMES + NUM_SENSOR_DATA);
// uint8_t dataIndex;


// Return prayer times
vector<dataItem> getTimesFromHTML(String payload) {
    // Perform some replacements to regex matching simpler
    payload.replace("Sol opp", "Sun");
    payload.replace("Sol ned.</span>", "Magrib");
    // Execute regular expression
    return runRegex(payload);
}


// Perform an HTTP GET and return an optional that includes the response
Optional<String> httpGET(String url) {
    Optional<String> ret;
    WiFiClient client;
    HTTPClient http;
    if (http.begin(client, url)) {
        // start connection and send HTTP header
        int httpCode = http.GET();
        // httpCode will be negative on error
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            // file found at server
            ret.value = http.getString();
            ret.success = true;
        }
        else {
            ret.err = http.errorToString(httpCode);
        }
        http.end();
    }
    else {
        ret.err = "[HTTP] Unable to connect";
    }
    return ret;
}


// Fetch times from the server. Returns true on success.
// bool updateTimes() {
//     // Wait for WiFi connection
//     if ((WiFiMulti.run() == WL_CONNECTED))
//     {
//         auto httpResponse = httpGET(url);
//         if (httpResponse.success) {
//             auto newDataVector = getTimesFromHTML(httpResponse.value);
//             if (newDataVector.size() > dataVector.size()) {
//                 Serial.printf("ERROR: Size mismatch. %u > %u\n", newDataVector.size(), dataVector.size());
//                 display.println("ERR 0");
//                 exit(1);
//             }
//             for (size_t i = 0; i < newDataVector.size(); i++) {
//                 dataVector.at(i) = newDataVector.at(i);
//             }
//         }
//         else {
//             Serial.printf("ERROR: HTTP request failed with error: %s\n", httpResponse.err.c_str());
//         }
//         return httpResponse.success;
//     }
//     return false;
// }

// Fetch times from the server. Returns true on success.
Optional < std::vector<dataItem>> getTimes() {
    Optional < std::vector<dataItem>> result;
    // Wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
        auto httpResponse = httpGET(url);
        if (httpResponse.success) {
            result.value = getTimesFromHTML(httpResponse.value);
        }
        else {
            display.println("HttpErr");
            Serial.printf("ERROR: HTTP request failed with error: %s\n", httpResponse.err.c_str());
            result.err = httpResponse.err;
        }
        result.success = httpResponse.success;
    }
    return result;
}

// Increment a number within the range of `max`
uint8_t increment(uint8_t i, size_t max) {
    if (i + 1 < uint16_t(max)) {
        return i + 1;
    }
    return 0;
}


void setup() {
    Serial.begin(115200);
    Wire.begin();

    /* Compile regular expression */
    setupRegex(regExpression);

    setupDisplay();
    setupSensor();
    setupSi7021();

    // WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
}


void loop() {
    
    auto times = getTimes();
    while (!times.success) {
        display.println("Loading...");
        display.display();
        resetDisplay();
        delay(1000);
    }

    displayDataVector(times.value, false);
    delay(10000);

    auto reading = getSensorReading();
    if (reading.valid) {
        vector<dataItem> sensorData(NUM_SENSOR_DATA);
      
        sensorData.at(CO2_INDEX).name = "CO2";
        sensorData.at(CO2_INDEX).value = reading.eco2;
        sensorData.at(CO2_INDEX).suffix = "p";
        sensorData.at(VOC_INDEX).name = "VOC";
        sensorData.at(VOC_INDEX).value = reading.etvoc;
        sensorData.at(VOC_INDEX).suffix = "p";
        yield();

        auto t = getTemperature();
        sensorData.at(TEMP_INDEX).name = "Temp";
        sensorData.at(TEMP_INDEX).value = floatToString(t);
        sensorData.at(TEMP_INDEX).suffix = (char)247;
        auto h = getHumidity();
        sensorData.at(HUMIDITY_INDEX).name = "Humidity";
        sensorData.at(HUMIDITY_INDEX).value = floatToString(h);
        sensorData.at(HUMIDITY_INDEX).suffix = "%";
        
        Serial.printf("CCS811: eco2=%u ppm  etvoc=%u ppb  ", reading.eco2, reading.etvoc);
        Serial.printf("Humidity: %.2f  Temp %.2f C\n", h, t);
        ccs811.set_envdata(t, h);

        displayDataVector(sensorData, true);
    }

    delay(10000);
}

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

using std::vector;

// Text looks like "<a href="#">Fajr</a><span>06:08</span></li>"
const char* regExpression = ">([a-zA-Z]+)</[span]{1,4}><span>([0-9]{2}:[0-9]{2})</span>";
const String url = "http://islam.no/prayer/get/85";
// const String url = "http://jigsaw.w3.org/HTTP/connection.html";

ESP8266WiFiMulti WiFiMulti;

#define NUM_PRAYER_TIMES 6
#define NUM_SENSOR_DATA 2
#define CO2_INDEX NUM_PRAYER_TIMES+0
#define VOC_INDEX NUM_PRAYER_TIMES+1

vector<dataPair> dataVector(NUM_PRAYER_TIMES + NUM_SENSOR_DATA);
uint8_t dataIndex;


// Return prayer times
vector<dataPair> getTimesFromHTML(String payload) {
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
bool updateTimes() {
    // Wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
        auto httpResponse = httpGET(url);
        if (httpResponse.success) {
            auto newDataVector = getTimesFromHTML(httpResponse.value);
            if (newDataVector.size() > dataVector.size()) {
                Serial.printf("ERROR: Size mismatch. %u > %u\n", newDataVector.size(), dataVector.size());
                exit(1);
            }
            for (size_t i = 0; i < newDataVector.size(); i++) {
                dataVector.at(i) = newDataVector.at(i);
            }
        }
        else {
            Serial.printf("ERROR: HTTP request failed with error: %s\n", httpResponse.err.c_str());
        }
        return httpResponse.success;
    }
    return false;
}

// Increment a number within the range of `max`
uint8_t increment(uint8_t i, size_t max) {
    if (i + 1 < uint16_t(max)) {
        return i + 1;
    }
    return 0;
}


void setup()
{
    Serial.begin(115200);

    /* Compile regular expression */
    setupRegex(regExpression);
    setupDisplay();
    setupSensor();

    dataVector.at(CO2_INDEX).name = "CO2";
    dataVector.at(VOC_INDEX).name = "VOC";

    // WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
}


void loop()
{
    while (!updateTimes()) {
        Serial.println("ERROR: Failed to update times");
        display.println("Loading...");
        display.display();
        resetDisplay();
        delay(1000);
    }

    if (dataIndex < dataVector.size()) {
        auto data = dataVector.at(dataIndex);
        display.println(data.name.c_str());
        display.println(data.value.c_str());
    }
    else {
        Serial.printf("ERROR: unexpected dataIndex value: %u\n", dataIndex);
    }
    dataIndex = increment(dataIndex, dataVector.size());

    auto reading = getSensorReading();
    if (reading.valid) {
        Serial.printf("CCS811: eco2=%u ppm  etvoc=%u ppb\n", reading.eco2, reading.etvoc);
        dataVector.at(CO2_INDEX).value = reading.eco2;
        dataVector.at(VOC_INDEX).value = reading.etvoc;
    }

    yield();
    display.display();
    display.clearDisplay();
    display.setCursor(0, 0);
    delay(2000);
}

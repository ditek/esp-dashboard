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

using std::vector;

// Text looks like "<a href="#">Fajr</a><span>06:08</span></li>"
const char* regExpression = ">([a-zA-Z]+)</[span]{1,4}><span>([0-9]{2}:[0-9]{2})</span>";
const String url = "http://islam.no/prayer/get/85";
// const String url = "http://jigsaw.w3.org/HTTP/connection.html";

ESP8266WiFiMulti WiFiMulti;
vector<matchPair> times;
uint8_t timeIndex;


// Return prayer times
vector<matchPair> getTimesFromHTML(String payload) {
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


// Fetch times from the server
void updateTimes() {
    // Wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
        auto httpResponse = httpGET(url);
        if (httpResponse.success) {
            times = getTimesFromHTML(httpResponse.value);
        }
        else {
            Serial.printf("ERROR: HTTP request failed with error: %s\n", httpResponse.err.c_str());
        }
    }
}

// Increment a number within the range of `max`
void increment(uint8_t* i, size_t max) {
    if (*i + 1 < uint16_t(max)) {
        *i += 1;
    }
    *i = 0;
}


void setup()
{
    Serial.begin(115200);

    /* Compile regular expression */
    setupRegex(regExpression);
    setupDisplay();

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
}


void loop()
{
    while (times.size() == 0) {
        updateTimes();
        delay(1000);
    }

    if (timeIndex < times.size()) {
        auto time = times.at(timeIndex);
        display.println(time.name.c_str());
        display.println(time.time.c_str());
    }
    else {
        Serial.printf("ERROR: unexpected timeIndex value: %u\n", timeIndex);
    }
    increment(&timeIndex, times.size());

    yield();
    display.display();
    display.clearDisplay();
    display.setCursor(0, 0);
    delay(2000);
}

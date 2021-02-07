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


// Return prayer times
vector<matchPair> getTimes(String payload) {
    // Perform some replacements to regex matching simpler
    payload.replace("Sol opp", "Sunrise");
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
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {
        auto httpResponse = httpGET(url);
        if (httpResponse.success) {
            auto matches = getTimes(httpResponse.value);
            for (auto&& pair : matches) {
                Serial.printf("%-10s %s\n", pair.name.c_str(), pair.time.c_str());
                display.printf("%-10s %s\n", pair.name.c_str(), pair.time.c_str());
            }
        }
        else {
            Serial.printf("ERROR: HTTP request failed with error: %s\n", httpResponse.err.c_str());
        }
    }

    yield();
    display.display();
    display.clearDisplay();
    display.setCursor(0, 0);
    delay(10000);
}

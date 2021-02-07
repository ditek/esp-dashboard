
#include <Arduino.h>
#include <regex.h>
#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Create from `config_template.h`
#include "config.h"
#include "types.hpp"
#include "display.h"

using std::vector;

#define MAX_REGEX_MATCHES 3

// Text looks like "<a href="#">Fajr</a><span>06:08</span></li>"
const char* regExpression = ">([a-zA-Z]+)</[span]{1,4}><span>([0-9]{2}:[0-9]{2})</span>";
const String url = "http://islam.no/prayer/get/85";
// const String url = "http://jigsaw.w3.org/HTTP/connection.html";

ESP8266WiFiMulti WiFiMulti;

regex_t regex;

typedef struct {
    String name;
    String time;
} matchPair;

typedef struct {
    matchPair pair;
    long next;
} regexSubMatch;


void setupRegex(const char* expression) {
    if (regcomp(&regex, expression, REG_EXTENDED)) {
        Serial.println("Could not compile regex");
        exit(1);
    }
}


regexSubMatch runRegexOnce(String payload) {
    regexSubMatch subMatch;
    // No match is indicated by subMatch.next == -1
    subMatch.next = -1;
    char payloadChar[payload.length() + 1];
    regmatch_t matches[MAX_REGEX_MATCHES];
    payload.toCharArray(payloadChar, payload.length());
    int reti = regexec(&regex, payloadChar, MAX_REGEX_MATCHES, matches, 0);
    if (!reti)
    {
        subMatch.next = matches[0].rm_eo;
        if (matches[0].rm_so == -1) {
            Serial.println("ERROR: Match not found");
            return subMatch;
        }
        // Ignore the first element as it contains the whole string
        // The second match is the name and the third is the time
        subMatch.pair.name = payload.substring(matches[1].rm_so, matches[1].rm_eo);
        subMatch.pair.time = payload.substring(matches[2].rm_so, matches[2].rm_eo);
    }
    else if (reti == REG_NOMATCH) {}
    else {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        Serial.printf("ERROR: Regex match failed: %s\n", msgbuf);
        exit(1);
    }
    return subMatch;
}


vector<matchPair> runRegex(String payload) {
    auto m = runRegexOnce(payload);
    vector<matchPair> matches;
    int pos = 0;
    while (m.next != -1) {
        // Append to the main vector
        matches.push_back(m.pair);
        pos += m.next;
        // Pick the string from the last match onward
        auto sub = payload.substring(pos);
        m = runRegexOnce(sub);
    }
    return matches;
}


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
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // httpCode will be negative on error
        if (httpCode <= 0) {
            ret.err = http.errorToString(httpCode);
        }
        else if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            // file found at server
            ret.value = http.getString();
            ret.success = true;
        }
        http.end();
    }
    else {
        ret.err = "[HTTP} Unable to connect";
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

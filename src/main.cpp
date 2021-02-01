
#include <Arduino.h>
#include <regex.h>
#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Create from `config_template.h`
#include "config.h"

using std::vector;

#define MAX_REGEX_MATCHES 3

const char* regExpression = ">([a-zA-Z]+)</[span]{1,4}><span>([0-9]{2}:[0-9]{2})</span>";
const String url = "http://islam.no/prayer/get/85";
// const String url = "http://jigsaw.w3.org/HTTP/connection.html";

ESP8266WiFiMulti WiFiMulti;

regex_t regex;

typedef struct {
    vector<String> matches;
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
    char payloadChar[payload.length() + 1];
    regmatch_t matches[MAX_REGEX_MATCHES];
    payload.toCharArray(payloadChar, payload.length());
    int reti = regexec(&regex, payloadChar, MAX_REGEX_MATCHES, matches, 0);
    if (!reti)
    {
        subMatch.next = matches[0].rm_eo;
        // Ignore the first element as it contains the whole string
        for (size_t i = 1; i < MAX_REGEX_MATCHES; i++)
        {
            auto match = matches[i];
            if (match.rm_so == -1) {
                break;
            }
            Serial.println(payload.substring(match.rm_so, match.rm_eo));
            subMatch.matches.push_back(payload.substring(match.rm_so, match.rm_eo));
        }
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


vector<String> runRegex(String payload) {
    auto m = runRegexOnce(payload);
    vector<String> matches;
    int pos = 0;
    while (m.matches.size() > 0) {
        // Append to the main vector
        matches.insert(matches.end(), m.matches.begin(), m.matches.end());
        pos += m.next;
        auto sub = payload.substring(pos);
        m = runRegexOnce(sub);
    }
    return matches;
}


void setup()
{
    Serial.begin(115200);

    Serial.println();
    Serial.println();
    Serial.println();

    /* Compile regular expression */
    setupRegex(regExpression);

    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
}


void loop()
{
    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED))
    {

        WiFiClient client;
        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        if (http.begin(client, url))
        { // HTTP

            Serial.print("[HTTP] GET...\n");
            // start connection and send HTTP header
            int httpCode = http.GET();

            // httpCode will be negative on error
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTP] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    String payload = http.getString();
                    // Serial.println(payload);

                    // Perform some replacements to regex matching simpler
                    payload.replace("Sol opp", "Sunrise");
                    payload.replace("Sol ned.</span>", "Magrib");

                    /* Execute regular expression */
                    auto matches = runRegex(payload);
                    Serial.println("*****************************");
                }
            }
            else
            {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }

            http.end();
        }
        else
        {
            Serial.printf("[HTTP} Unable to connect\n");
        }
    }

    delay(10000);
}

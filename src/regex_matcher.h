#pragma once

#include <Arduino.h>
#include <regex.h>
#include <vector>

using std::vector;

#define MAX_REGEX_MATCHES 3

struct dataPair{
    String name;
    String value;
};

struct regexSubMatch {
    dataPair pair;
    long next;
};

void setupRegex(const char* expression);
vector<dataPair> runRegex(String payload);

#pragma once

#include <Arduino.h>
#include <regex.h>
#include <vector>

using std::vector;

#define MAX_REGEX_MATCHES 3

typedef struct {
    String name;
    String value;
} dataPair;

typedef struct {
    dataPair pair;
    long next;
} regexSubMatch;

void setupRegex(const char* expression);
vector<dataPair> runRegex(String payload);

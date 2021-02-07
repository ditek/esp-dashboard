#pragma once

#include <Arduino.h>
#include <regex.h>
#include <vector>

using std::vector;

#define MAX_REGEX_MATCHES 3

typedef struct {
    String name;
    String time;
} matchPair;

typedef struct {
    matchPair pair;
    long next;
} regexSubMatch;

void setupRegex(const char* expression);
vector<matchPair> runRegex(String payload);

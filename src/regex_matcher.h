#pragma once

#include <Arduino.h>
#include <regex.h>
#include <vector>
#include "types.hpp"

using std::vector;

#define MAX_REGEX_MATCHES 3

struct regexSubMatch {
    dataItem pair;
    long next;
};

void setupRegex(const char* expression);
vector<dataItem> runRegex(String payload);

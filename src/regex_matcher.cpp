#include "regex_matcher.h"

regex_t regex;

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
        subMatch.pair.value = payload.substring(matches[2].rm_so, matches[2].rm_eo);
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


vector<dataPair> runRegex(String payload) {
    auto m = runRegexOnce(payload);
    vector<dataPair> matches;
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

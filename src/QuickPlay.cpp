#include <Arduino.h>
#include <HTTPClient.h>
#include "QuickPlay.h"

void quickPlay(int id) {
    HTTPClient http;
    String url = String("http://arrow.local/quickplay/") + id;
    http.begin(url);
    int code = http.POST("");
    if (code > 0) {
        Serial.printf("[QuickPlay] POST %s -> %d\n", url.c_str(), code);
    } else {
        Serial.printf("[QuickPlay] POST %s failed: %s\n",
            url.c_str(), HTTPClient::errorToString(code).c_str());
    }
    http.end();
}

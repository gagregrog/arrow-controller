#include <Arduino.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include "QuickPlay.h"

void quickPlay(int id) {
    IPAddress ip = MDNS.queryHost("arrow");
    if ((uint32_t)ip == 0) {
        Serial.println("[QuickPlay] mDNS resolution failed for arrow.local");
        return;
    }

    HTTPClient http;
    String url = String("http://") + ip.toString() + "/quickplay/" + id;
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

#include <Arduino.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include "ArrowClient.h"

static const char* ARROW_HOST = "arrow";
static const int ARROW_PORT = 8000;

static String arrowBaseUrl() {
    IPAddress ip = MDNS.queryHost(ARROW_HOST);
    if ((uint32_t)ip == 0) {
        Serial.printf("[ArrowClient] mDNS resolution failed for %s.local\n", ARROW_HOST);
        return "";
    }
    return String("http://") + ip.toString() + ":" + ARROW_PORT;
}

static int post(const String& path) {
    String base = arrowBaseUrl();
    if (base.isEmpty()) return -1;

    HTTPClient http;
    String url = base + path;
    http.begin(url);
    int code = http.POST("");
    if (code > 0) {
        Serial.printf("[ArrowClient] POST %s -> %d\n", url.c_str(), code);
    } else {
        Serial.printf("[ArrowClient] POST %s failed: %s\n",
            url.c_str(), HTTPClient::errorToString(code).c_str());
    }
    http.end();
    return code;
}

int arrowQuickPlay(int index) { return post("/quickplay/" + String(index)); }
int arrowPlay()               { return post("/play"); }
int arrowStop()               { return post("/stop"); }
int arrowShuffle()            { return post("/shuffle"); }
int arrowNextTrack()          { return post("/next"); }
int arrowPreviousTrack()      { return post("/previous"); }
int arrowRestartTrack()       { return post("/restart"); }
int arrowRestartMopidy()      { return post("/service/mopidy/restart"); }

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

static String get(const String& path) {
    String base = arrowBaseUrl();
    if (base.isEmpty()) return "";
    HTTPClient http;
    http.begin(base + path);
    http.setTimeout(3000);
    int code = http.GET();
    if (code != 200) {
        Serial.printf("[ArrowClient] GET %s -> %d\n", path.c_str(), code);
        http.end();
        return "";
    }
    String result = http.getString();
    http.end();
    return result;
}

static int put(const String& path, const String& body) {
    String base = arrowBaseUrl();
    if (base.isEmpty()) return -1;
    HTTPClient http;
    http.begin(base + path);
    http.setTimeout(3000);
    http.addHeader("Content-Type", "application/json");
    int code = http.PUT(body);
    Serial.printf("[ArrowClient] PUT %s -> %d\n", path.c_str(), code);
    http.end();
    return code;
}

String arrowGetQuickplay()                             { return get("/quickplay"); }
String arrowGetArtists()                               { return get("/artists"); }
String arrowGetArtistAlbums(const String& encodedArtist) { return get("/artist/" + encodedArtist + "/albums"); }
int    arrowPutQuickplay(int index, const String& body) { return put("/quickplay/" + String(index), body); }
int    arrowClearQuickplay()                            { return put("/quickplay", "[]"); }

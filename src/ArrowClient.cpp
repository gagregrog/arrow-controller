#include <Arduino.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
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

// POST that returns the response body (not just the status code). Used where
// the Pi replies with JSON we need — e.g. the sensor sample burst, which can
// take a couple of seconds, so the timeout is caller-supplied.
static String postForBody(const String& path, int timeoutMs) {
    String base = arrowBaseUrl();
    if (base.isEmpty()) return "";
    HTTPClient http;
    http.begin(base + path);
    http.setTimeout(timeoutMs);
    int code = http.POST("");
    String result = (code == 200) ? http.getString() : "";
    if (code != 200) {
        Serial.printf("[ArrowClient] POST %s -> %d\n", path.c_str(), code);
    }
    http.end();
    return result;
}

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
String arrowGetIR()                                    { return get("/ir"); }
int    arrowSendIR(const String& function, int count) {
    String path = "/ir/" + function;
    if (count > 1) path += "?count=" + String(count);
    return post(path);
}
// These return immediately — the Pi acknowledges and sends the IR presses in
// the background, so there's no long connection to hold open.
int    arrowFloorVolume()     { return post("/volume/floor"); }
int    arrowStartupVolume()   { return post("/volume/startup"); }
int    arrowStereoOff(const String& inputCmd) {
    String path = "/stereo/off";
    if (inputCmd.length()) path += "?input_cmd=" + inputCmd;
    return post(path);
}

String arrowGetStereo()       { return get("/stereo"); }
String arrowGetStereoConfig() { return get("/stereo/config"); }
int    arrowPutStereoConfig(const String& body) { return put("/stereo/config", body); }
int    arrowRebootPi()        { return post("/system/reboot"); }
int    arrowShutdownPi()      { return post("/system/shutdown"); }

String arrowStereoSample(int count) {
    // The Pi takes ~100 readings (a few seconds); allow generous headroom.
    return postForBody("/stereo/sample?count=" + String(count), 10000);
}

StereoStatus arrowStereoStatus() {
    String body = arrowGetStereo();
    if (body.isEmpty()) return STEREO_UNKNOWN;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok) {
        Serial.println("[ArrowClient] /stereo: JSON parse failed");
        return STEREO_UNKNOWN;
    }
    // "on" is true/false when the sensor has a reading, null when unavailable.
    if (doc["on"].isNull()) return STEREO_UNKNOWN;
    return doc["on"].as<bool>() ? STEREO_ON : STEREO_OFF;
}

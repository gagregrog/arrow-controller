#include "web/WebSocket.h"
#include "web/API.h"
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>

static AsyncWebSocket _ws("/ws");

static String uidToString(const uint8_t* uid, uint8_t len) {
    String s;
    for (uint8_t i = 0; i < len; i++) {
        if (i > 0) s += ':';
        if (uid[i] < 0x10) s += '0';
        s += String(uid[i], HEX);
    }
    s.toUpperCase();
    return s;
}

void wsBegin() {
    apiGetServer()->addHandler(&_ws);
}

void wsLoop() {
    _ws.cleanupClients();
}

void wsNotifyBadgeScan(int index) {
    if (_ws.count() == 0) return;
    JsonDocument doc;
    doc["type"] = "badge_scan";
    doc["index"] = index;
    String msg;
    serializeJson(doc, msg);
    _ws.textAll(msg);
}

void wsNotifyUnknownBadge(const uint8_t* uid, uint8_t len) {
    if (_ws.count() == 0) return;
    JsonDocument doc;
    doc["type"] = "unknown_badge";
    doc["uid"] = uidToString(uid, len);
    String msg;
    serializeJson(doc, msg);
    _ws.textAll(msg);
}

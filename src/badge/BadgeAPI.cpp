#include "badge/BadgeAPI.h"
#include "web/API.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

static BadgeStore* _store = nullptr;
static String _pendingBody;

static String uidToHex(const BadgeUID& b) {
    String s;
    for (uint8_t i = 0; i < b.len; i++) {
        if (i > 0) s += ':';
        if (b.bytes[i] < 0x10) s += '0';
        s += String(b.bytes[i], HEX);
    }
    s.toUpperCase();
    return s;
}

static BadgeUID uidFromHex(const char* s) {
    BadgeUID b = {};
    uint8_t i = 0;
    while (*s && i < 7) {
        b.bytes[i++] = (uint8_t)strtoul(s, nullptr, 16);
        while (*s && *s != ':') s++;
        if (*s == ':') s++;
    }
    b.len = i;
    return b;
}

void badgeAPIBegin(BadgeStore* store) {
    _store = store;

    apiGetServer()->on("/api/badges", HTTP_GET, [](AsyncWebServerRequest* req) {
        JsonDocument doc;
        JsonArray arr = doc["badges"].to<JsonArray>();
        for (int i = 0; i < _store->count(); i++) {
            arr.add(uidToHex(_store->get(i)));
        }
        String body;
        serializeJson(doc, body);
        req->send(200, "application/json", body);
    });

    // Body handler must be passed inline — onRequestBody only fires for unmatched routes.
    apiGetServer()->on("/api/badges", HTTP_POST,
        [](AsyncWebServerRequest* req) {
            JsonDocument doc;
            if (deserializeJson(doc, _pendingBody) != DeserializationError::Ok
                || !doc["uid"].is<const char*>()) {
                req->send(400, "application/json", "{\"error\":\"invalid body\"}");
                return;
            }
            BadgeUID b = uidFromHex(doc["uid"].as<const char*>());
            if (b.len == 0) {
                req->send(400, "application/json", "{\"error\":\"invalid uid\"}");
                return;
            }
            if (_store->lookup(b.bytes, b.len) >= 0) {
                req->send(409, "application/json", "{\"error\":\"already registered\"}");
                return;
            }
            int index = _store->add(b.bytes, b.len);
            JsonDocument resp;
            resp["index"] = index;
            resp["uid"] = doc["uid"];
            String body;
            serializeJson(resp, body);
            req->send(201, "application/json", body);
        },
        nullptr,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
            if (index == 0) _pendingBody.clear();
            _pendingBody += String((char*)data, len);
        }
    );

    apiAddNotFoundHandler([](AsyncWebServerRequest* req) -> bool {
        String url = req->url();
        if (req->method() == HTTP_DELETE && url.startsWith("/api/badges/")) {
            int idx = url.substring(12).toInt();
            if (_store->remove(idx)) {
                req->send(200, "application/json", "{\"ok\":true}");
            } else {
                req->send(404, "application/json", "{\"error\":\"not found\"}");
            }
            return true;
        }
        return false;
    });
}

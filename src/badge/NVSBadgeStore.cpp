#include "badge/NVSBadgeStore.h"
#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>

static const char* NS  = "badges";
static const char* KEY = "uids";

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

void NVSBadgeStore::load() {
    Preferences prefs;
    prefs.begin(NS, true);
    String json = prefs.getString(KEY, "[]");
    prefs.end();

    JsonDocument doc;
    if (deserializeJson(doc, json) != DeserializationError::Ok) return;
    _badges.clear();
    for (JsonVariant v : doc.as<JsonArray>()) {
        _badges.push_back(uidFromHex(v.as<const char*>()));
    }
    Serial.printf("[BadgeStore] Loaded %d badge(s)\n", (int)_badges.size());
}

void NVSBadgeStore::save() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (auto& b : _badges) arr.add(uidToHex(b));

    String json;
    serializeJson(doc, json);

    Preferences prefs;
    prefs.begin(NS, false);
    prefs.putString(KEY, json);
    prefs.end();
}

int NVSBadgeStore::lookup(const uint8_t* uid, uint8_t len) {
    for (int i = 0; i < (int)_badges.size(); i++) {
        if (_badges[i].len == len && memcmp(_badges[i].bytes, uid, len) == 0)
            return i;
    }
    return -1;
}

int NVSBadgeStore::count() {
    return (int)_badges.size();
}

BadgeUID NVSBadgeStore::get(int index) {
    return _badges[index];
}

int NVSBadgeStore::add(const uint8_t* uid, uint8_t len) {
    BadgeUID b = {};
    memcpy(b.bytes, uid, len);
    b.len = len;
    _badges.push_back(b);
    save();
    Serial.printf("[BadgeStore] Registered badge %d\n", (int)_badges.size() - 1);
    return (int)_badges.size() - 1;
}

bool NVSBadgeStore::remove(int index) {
    if (index < 0 || index >= (int)_badges.size()) return false;
    _badges.erase(_badges.begin() + index);
    save();
    Serial.printf("[BadgeStore] Removed badge %d\n", index);
    return true;
}

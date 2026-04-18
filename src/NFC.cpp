#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "pins.h"
#include "TagMap.h"
#include "NFC.h"

static Adafruit_PN532 nfc(PIN_NFC_IRQ, PIN_NFC_RESET);

static const unsigned long COOLDOWN_MS = 3000;
static unsigned long lastReadMs = 0;
static uint8_t lastUID[7] = {};
static uint8_t lastUIDLen = 0;

static int lookupTag(const uint8_t *uid, uint8_t uidLen) {
    for (int i = 0; i < TAG_MAP_SIZE; i++) {
        if (uidLen == TAG_MAP[i].uidLen &&
            memcmp(uid, TAG_MAP[i].uid, uidLen) == 0) {
            return TAG_MAP[i].id;
        }
    }
    return -1;
}

static void logUID(const uint8_t *uid, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (i > 0) Serial.print(":");
        if (uid[i] < 0x10) Serial.print("0");
        Serial.print(uid[i], HEX);
    }
}

void nfcBegin() {
    Wire.begin(PIN_SDA, PIN_SCL);
    nfc.begin();

    uint32_t ver = nfc.getFirmwareVersion();
    if (!ver) {
        Serial.println("[NFC] PN532 not found — check wiring");
        while (true);
    }
    Serial.printf("[NFC] PN532 firmware v%d.%d\n", (ver >> 16) & 0xFF, (ver >> 8) & 0xFF);
    nfc.SAMConfig();
    Serial.println("[NFC] Ready");
}

int nfcLoop() {
    uint8_t uid[7];
    uint8_t uidLen;

    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 100)) {
        return -1;
    }

    unsigned long now = millis();
    if (uidLen == lastUIDLen &&
        memcmp(uid, lastUID, uidLen) == 0 &&
        (now - lastReadMs) < COOLDOWN_MS) {
        return -1;
    }

    lastReadMs = now;
    lastUIDLen = uidLen;
    memcpy(lastUID, uid, uidLen);

    Serial.print("[NFC] Tag UID: ");
    logUID(uid, uidLen);
    Serial.println();

    int id = lookupTag(uid, uidLen);
    if (id < 0) {
        Serial.println("[NFC]   -> unknown tag, add UID to tags.csv");
        return -1;
    }

    Serial.printf("[NFC]   -> badge %d\n", id);
    return id;
}

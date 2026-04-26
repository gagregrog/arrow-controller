#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "NFC.h"
#include "ArrowClient.h"
#include "Buttons.h"
#include "Leds.h"
#include "web/API.h"
#include "web/WebSocket.h"
#include "web/WebUI.h"
#include "badge/NVSBadgeStore.h"
#include "badge/BadgeAPI.h"

static NVSBadgeStore badgeStore;

void setup() {
    Serial.begin(115200);
    badgeStore.load();

    ledsBegin();

    WiFiManager wm;
    wm.setAPCallback([](WiFiManager*) { ledsSetState(LED_STATE_WIFI_PORTAL); });
    wm.autoConnect("ArrowController");
    ledsSetState(LED_STATE_NORMAL);
    // Power down the radio between DTIM beacon intervals (~100ms) while
    // staying associated — cuts idle draw without adding reconnect latency.
    WiFi.setSleep(true);
    Serial.println("[WiFi] Connected");

    MDNS.begin("arrow-controller");
    MDNS.addService("http", "tcp", 80);

    ArduinoOTA.setHostname("arrow-controller");
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
    Serial.println("[OTA] Ready");

    apiInit();
    badgeAPIBegin(&badgeStore);
    wsBegin();
    webUIBegin();
    apiStart();

    nfcBegin(&badgeStore);
    buttonsBegin();
}

void loop() {
    ArduinoOTA.handle();
    wsLoop();

    ledsSetState(WiFi.status() == WL_CONNECTED ? LED_STATE_NORMAL : LED_STATE_WIFI_LOST);

    int id = nfcLoop();
    if (id >= 0) {
        ledsFlashBadge();
        wsNotifyBadgeScan(id);
        int code = arrowQuickPlay(id);
        if (code < 200 || code >= 300) ledsFlashError();
    } else if (id == NFC_UNKNOWN_TAG) {
        ledsFlashUnknown();
        wsNotifyUnknownBadge(nfcLastUID(), nfcLastUIDLen());
    }
    buttonsLoop();
}

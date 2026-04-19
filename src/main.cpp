#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "NFC.h"
#include "ArrowClient.h"
#include "Buttons.h"
#include "Leds.h"

void setup() {
    Serial.begin(115200);

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

    nfcBegin();
    buttonsBegin();
}

void loop() {
    ledsSetState(WiFi.status() == WL_CONNECTED ? LED_STATE_NORMAL : LED_STATE_WIFI_LOST);

    int id = nfcLoop();
    if (id >= 0) {
        ledsFlashBadge();
        int code = arrowQuickPlay(id);
        if (code < 200 || code >= 300) ledsFlashError();
    }
    buttonsLoop();
}

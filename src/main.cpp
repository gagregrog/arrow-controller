#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "NFC.h"
#include "ArrowClient.h"
#include "Buttons.h"

void setup() {
    Serial.begin(115200);

    WiFiManager wm;
    wm.autoConnect("ArrowController");
    // Power down the radio between DTIM beacon intervals (~100ms) while
    // staying associated — cuts idle draw without adding reconnect latency.
    WiFi.setSleep(true);
    Serial.println("[WiFi] Connected");

    MDNS.begin("arrow-controller");

    nfcBegin();
    buttonsBegin();
}

void loop() {
    int id = nfcLoop();
    if (id >= 0) {
        arrowQuickPlay(id);
    }
    buttonsLoop();
}

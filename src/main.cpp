#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include "NFC.h"
#include "ArrowClient.h"

void setup() {
    Serial.begin(115200);

    WiFiManager wm;
    wm.autoConnect("ArrowController");
    Serial.println("[WiFi] Connected");

    MDNS.begin("arrow-controller");

    nfcBegin();
}

void loop() {
    int id = nfcLoop();
    if (id >= 0) {
        arrowQuickPlay(id);
    }
}

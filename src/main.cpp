#include <Arduino.h>
#include <WiFiManager.h>
#include "NFC.h"
#include "QuickPlay.h"

void setup() {
    Serial.begin(115200);

    WiFiManager wm;
    wm.autoConnect("ArrowController");
    Serial.println("[WiFi] Connected");

    nfcBegin();
}

void loop() {
    int id = nfcLoop();
    if (id >= 0) {
        quickPlay(id);
    }
}

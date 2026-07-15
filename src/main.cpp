#include <Arduino.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "NFC.h"
#include "ArrowClient.h"
#include "WiFiConn.h"
#include "Buttons.h"
#include "Leds.h"
#include "ScanCooldown.h"
#include "web/API.h"
#include "web/WebSocket.h"
#include "web/WebUI.h"
#include "badge/NVSBadgeStore.h"
#include "badge/BadgeAPI.h"

static NVSBadgeStore badgeStore;

// After a badge triggers playback, ignore further scans for this long. Scanning
// during the window shows purple breathing until it ends.
static const uint32_t SCAN_COOLDOWN_MS = 10000;
static ScanCooldown   scanCooldown(SCAN_COOLDOWN_MS);

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
    // First line of defense against drops; wifiReconnectLoop() handles outages
    // the core's auto-reconnect gives up on.
    WiFi.setAutoReconnect(true);
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

    uint32_t now = millis();

    wifiReconnectLoop(now);

    int id = nfcLoop();
    if (id >= 0) {
        // Global lockout: a badge only plays if we're not still cooling down
        // from a previous scan. Rejected scans fall through to the breathing
        // cue set via the LED state below.
        if (scanCooldown.tryScan(now)) {
            ledsFlashBadge();
            wsNotifyBadgeScan(id);
            int code = arrowQuickPlay(id);
            if (code < 200 || code >= 300) ledsFlashError();
        }
    } else if (id == NFC_UNKNOWN_TAG) {
        ledsFlashUnknown();
        wsNotifyUnknownBadge(nfcLastUID(), nfcLastUIDLen());
    }

    // LED base state: WiFi trouble wins, then the scan-cooldown lockout cue,
    // otherwise idle. Re-evaluated every loop so breathing clears itself when
    // the window ends.
    LEDState ledState;
    if (WiFi.status() != WL_CONNECTED)    ledState = LED_STATE_WIFI_LOST;
    else if (scanCooldown.breathing(now)) ledState = LED_STATE_COOLDOWN;
    else                                  ledState = LED_STATE_NORMAL;
    ledsSetState(ledState);

    buttonsLoop();
}

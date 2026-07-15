#include "WiFiConn.h"
#include <WiFi.h>
#include "ReconnectTimer.h"

// How long to wait between reconnect attempts while the link is down. The first
// attempt fires immediately on the drop; this only paces the retries after that.
static const uint32_t WIFI_RECONNECT_INTERVAL_MS = 10000;
static ReconnectTimer reconnectTimer(WIFI_RECONNECT_INTERVAL_MS);

void wifiReconnectLoop(uint32_t now) {
    bool connected = WiFi.status() == WL_CONNECTED;
    if (reconnectTimer.shouldAttempt(now, connected)) {
        Serial.println("[WiFi] Link down — attempting reconnect");
        WiFi.reconnect();
    }
}

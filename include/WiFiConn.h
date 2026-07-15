#pragma once
#include <stdint.h>

// Drive from loop() with millis(). While the STA connection is down, triggers a
// WiFi.reconnect() on the first detected drop and then periodically until the
// link is restored. No-op while connected.
void wifiReconnectLoop(uint32_t now);

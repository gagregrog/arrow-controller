#pragma once
#include <stdint.h>

// Decides when to trigger a WiFi reconnect attempt while the link is down. Fires
// immediately on the first detected drop, then no more often than once per
// interval until the link is restored. Resets when connected, so a later drop
// again retries right away. Pure logic — the caller supplies `now` (millis())
// and the current connection state, so it is testable without hardware.
class ReconnectTimer {
public:
    explicit ReconnectTimer(uint32_t intervalMs)
        : _intervalMs(intervalMs), _lastAttempt(0), _pending(false) {}

    // Call every loop. Returns true when the caller should attempt a reconnect
    // now. While disconnected, returns true on the first call and then again
    // once `intervalMs` has elapsed since the last attempt.
    bool shouldAttempt(uint32_t now, bool connected) {
        if (connected) {
            _pending = false;
            return false;
        }
        if (!_pending || (now - _lastAttempt) >= _intervalMs) {
            _pending     = true;
            _lastAttempt = now;
            return true;
        }
        return false;
    }

private:
    uint32_t _intervalMs;
    uint32_t _lastAttempt;
    bool     _pending;   // true once we've started retrying this outage
};

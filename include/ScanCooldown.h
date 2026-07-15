#pragma once
#include <stdint.h>

// Global scan lockout: after a badge is scanned, further scans are rejected for
// a fixed period. The window is anchored to the accepted scan and is NOT
// extended by scans that arrive (and are rejected) during it. Pure logic — the
// caller supplies `now` (millis()), so it is testable without hardware.
class ScanCooldown {
public:
    explicit ScanCooldown(uint32_t periodMs)
        : _periodMs(periodMs), _until(0), _armed(false), _rejected(false) {}

    // Register a scan attempt. Returns true if accepted (no active cooldown),
    // starting a fresh window; false if rejected (still cooling down).
    bool tryScan(uint32_t now) {
        if (_armed && now < _until) {
            _rejected = true;   // a scan hit the lockout — arm the breathing cue
            return false;
        }
        _armed    = true;
        _until    = now + _periodMs;
        _rejected = false;
        return true;
    }

    // True only while a scan has been rejected during the still-active window —
    // i.e. show the "locked out, wait" breathing. Clears itself when the window
    // ends or the next scan is accepted.
    bool breathing(uint32_t now) const {
        return _rejected && _armed && now < _until;
    }

private:
    uint32_t _periodMs;
    uint32_t _until;
    bool     _armed;
    bool     _rejected;
};

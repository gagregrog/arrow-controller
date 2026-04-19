#include <Arduino.h>
#include "Buttons.h"
#include "ArrowClient.h"
#include "pins.h"

#define DEBOUNCE_MS 50

static void onPlay()          { arrowPlay(); }
static void onStop()          { arrowStop(); }
static void onPrevious()      { arrowPreviousTrack(); }
static void onNext()          { arrowNextTrack(); }
static void onRestart()       { arrowRestartTrack(); }
static void onMopidyRestart() { arrowRestartMopidy(); }

struct ButtonState {
    int pin;
    void (*onPress)();
    bool lastRaw;
    bool stable;
    unsigned long lastChangeMs;
};

static ButtonState buttons[] = {
    { PIN_BTN_PLAY,           onPlay,          HIGH, HIGH, 0 },
    { PIN_BTN_STOP,           onStop,          HIGH, HIGH, 0 },
    { PIN_BTN_PREVIOUS,       onPrevious,      HIGH, HIGH, 0 },
    { PIN_BTN_NEXT,           onNext,          HIGH, HIGH, 0 },
    { PIN_BTN_RESTART,        onRestart,       HIGH, HIGH, 0 },
    { PIN_BTN_MOPIDY_RESTART, onMopidyRestart, HIGH, HIGH, 0 },
};

static const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

void buttonsBegin() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        pinMode(buttons[i].pin, INPUT_PULLUP);
    }
}

void buttonsLoop() {
    unsigned long now = millis();
    for (int i = 0; i < BUTTON_COUNT; i++) {
        ButtonState& b = buttons[i];
        bool raw = digitalRead(b.pin);
        if (raw != b.lastRaw) {
            b.lastRaw = raw;
            b.lastChangeMs = now;
        }
        if ((now - b.lastChangeMs) >= DEBOUNCE_MS && raw != b.stable) {
            b.stable = raw;
            if (raw == LOW) {
                b.onPress();
            }
        }
    }
}

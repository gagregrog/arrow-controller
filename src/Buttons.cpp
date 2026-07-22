#include <Arduino.h>
#include "Buttons.h"
#include "ArrowClient.h"
#include "Leds.h"
#include "pins.h"

#define DEBOUNCE_MS 50
#define LONG_PRESS_MS 1500

static bool failed(int code) { return code < 200 || code >= 300; }

// Play: pause when playing, resume when paused, else start quickplay #0.
// The server (/play) decides which, based on playback state.
static void onPlay() {
    ledsFlashPlay();
    if (failed(arrowPlay())) ledsFlashError();
}
// Play long-press: shuffle the whole library.
static void onShuffle() {
    ledsFlashPlay();
    if (failed(arrowShuffle())) ledsFlashError();
}
// Stop: stop playback and switch the receiver back to the TV input.
static void onStop() {
    ledsFlashStop();
    int stop = arrowStop();
    int tv   = arrowSendIR("tv");
    if (failed(stop) || failed(tv)) ledsFlashError();
}
// Stop long-press: stop playback, switch the receiver back to TV, then have the
// Pi floor the volume and power the receiver off (in that order). The Pi runs
// that sequence in the background and acks immediately, so we don't block here;
// flooring first means the stereo comes back up quiet next time.
static void onStopPower() {
    ledsFlashStop();
    int stop = arrowStop();
    int tv   = arrowSendIR("tv");
    int off  = arrowStereoOff();
    if (failed(stop) || failed(tv) || failed(off)) ledsFlashError();
}
static void onPrevious() {
    ledsFlashTrack();
    if (failed(arrowPreviousTrack())) ledsFlashError();
}
// Previous long-press: turn the volume down 3 increments (one request).
static void onVolumeDown() {
    ledsFlashTrack();
    if (failed(arrowSendIR("volumeDown", 3))) ledsFlashError();
}
static void onNext() {
    ledsFlashTrack();
    if (failed(arrowNextTrack())) ledsFlashError();
}
// Next long-press: turn the volume up 3 increments (one request).
static void onVolumeUp() {
    ledsFlashTrack();
    if (failed(arrowSendIR("volumeUp", 3))) ledsFlashError();
}

struct ButtonDef {
    int pin;
    void (*onPress)();
    void (*onLongPress)();  // null = no long press action
};

static const ButtonDef buttons[] = {
    { PIN_BTN_PLAY,     onPlay,     onShuffle    },
    { PIN_BTN_STOP,     onStop,     onStopPower  },
    { PIN_BTN_PREVIOUS, onPrevious, onVolumeDown },
    { PIN_BTN_NEXT,     onNext,     onVolumeUp   },
};

static const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// Set by ISR on falling edge; cleared by buttonsLoop after debounce
static volatile unsigned long isrTriggerMs[BUTTON_COUNT];
// For buttons with onLongPress: records confirmed press time and tracks awaiting release
static unsigned long pressedMs[BUTTON_COUNT];
static bool waitingRelease[BUTTON_COUNT];

static void IRAM_ATTR isr0() { isrTriggerMs[0] = millis(); }
static void IRAM_ATTR isr1() { isrTriggerMs[1] = millis(); }
static void IRAM_ATTR isr2() { isrTriggerMs[2] = millis(); }
static void IRAM_ATTR isr3() { isrTriggerMs[3] = millis(); }

void buttonsBegin() {
    static void (*isrs[BUTTON_COUNT])() = { isr0, isr1, isr2, isr3 };
    for (int i = 0; i < BUTTON_COUNT; i++) {
        isrTriggerMs[i] = 0;
        pressedMs[i] = 0;
        waitingRelease[i] = false;
        pinMode(buttons[i].pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttons[i].pin), isrs[i], FALLING);
    }
}

void buttonsLoop() {
    unsigned long now = millis();
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (waitingRelease[i]) {
            if ((now - pressedMs[i]) >= LONG_PRESS_MS) {
                waitingRelease[i] = false;
                buttons[i].onLongPress();
            } else if (digitalRead(buttons[i].pin) == HIGH) {
                waitingRelease[i] = false;
                buttons[i].onPress();
            }
            continue;
        }

        unsigned long t = isrTriggerMs[i];
        if (t != 0 && (now - t) >= DEBOUNCE_MS) {
            isrTriggerMs[i] = 0;
            if (digitalRead(buttons[i].pin) == LOW) {
                if (buttons[i].onLongPress != nullptr) {
                    pressedMs[i] = t;
                    waitingRelease[i] = true;
                } else {
                    buttons[i].onPress();
                }
            }
        }
    }
}

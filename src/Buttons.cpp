#include <Arduino.h>
#include "Buttons.h"
#include "ArrowClient.h"
#include "Leds.h"
#include "pins.h"

#define DEBOUNCE_MS 50

static bool failed(int code) { return code < 200 || code >= 300; }

static void onPlay() {
    ledsFlashPlay();
    if (failed(arrowPlay())) ledsFlashError();
}
static void onStop() {
    ledsFlashStop();
    if (failed(arrowStop())) ledsFlashError();
}
static void onPrevious() {
    ledsFlashTrack();
    if (failed(arrowPreviousTrack())) ledsFlashError();
}
static void onNext() {
    ledsFlashTrack();
    if (failed(arrowNextTrack())) ledsFlashError();
}
static void onRestart() {
    ledsFlashTrack();
    if (failed(arrowRestartTrack())) ledsFlashError();
}
static void onMopidyRestart() {
    ledsFlashMopidyRestart();
    if (failed(arrowRestartMopidy())) ledsFlashError();
}

struct ButtonDef {
    int pin;
    void (*onPress)();
};

static const ButtonDef buttons[] = {
    { PIN_BTN_PLAY,           onPlay          },
    { PIN_BTN_STOP,           onStop          },
    { PIN_BTN_PREVIOUS,       onPrevious      },
    { PIN_BTN_NEXT,           onNext          },
    { PIN_BTN_RESTART,        onRestart       },
    { PIN_BTN_MOPIDY_RESTART, onMopidyRestart },
};

static const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

// Set by ISR on falling edge; cleared by buttonsLoop after debounce
static volatile unsigned long isrTriggerMs[BUTTON_COUNT];

static void IRAM_ATTR isr0() { isrTriggerMs[0] = millis(); }
static void IRAM_ATTR isr1() { isrTriggerMs[1] = millis(); }
static void IRAM_ATTR isr2() { isrTriggerMs[2] = millis(); }
static void IRAM_ATTR isr3() { isrTriggerMs[3] = millis(); }
static void IRAM_ATTR isr4() { isrTriggerMs[4] = millis(); }
static void IRAM_ATTR isr5() { isrTriggerMs[5] = millis(); }

void buttonsBegin() {
    static void (*isrs[BUTTON_COUNT])() = { isr0, isr1, isr2, isr3, isr4, isr5 };
    for (int i = 0; i < BUTTON_COUNT; i++) {
        isrTriggerMs[i] = 0;
        pinMode(buttons[i].pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(buttons[i].pin), isrs[i], FALLING);
    }
}

void buttonsLoop() {
    unsigned long now = millis();
    for (int i = 0; i < BUTTON_COUNT; i++) {
        unsigned long t = isrTriggerMs[i];
        if (t != 0 && (now - t) >= DEBOUNCE_MS) {
            isrTriggerMs[i] = 0;
            if (digitalRead(buttons[i].pin) == LOW) {
                buttons[i].onPress();
            }
        }
    }
}

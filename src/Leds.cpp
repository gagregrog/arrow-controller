#include <Arduino.h>
#include <FastLED.h>
#include "Leds.h"
#include "pins.h"

#define NUM_LEDS          12
#define LED_TYPE          WS2811
#define COLOR_ORDER       GRB
#define BREATHE_PERIOD_MS 3000
#define BREATHE_FPS_MS    33    // ~30fps
#define FLASH_ON_MS       80
#define FLASH_OFF_MS      80
#define LED_MAX_BRIGHTNESS 128  // 0–255

static const CRGB ORANGE = CRGB(255,  80,   0);
static const CRGB BLUE   = CRGB(  0, 100, 255);
static const CRGB GREEN  = CRGB(  0, 255,   0);
static const CRGB PURPLE = CRGB(128,   0, 255);
static const CRGB RED    = CRGB(255,   0,   0);

// Shared state — written by main-loop calls, read by LED task.
// All access protected by mux.
static portMUX_TYPE mux         = portMUX_INITIALIZER_UNLOCKED;
static LEDState     sharedState = LED_STATE_NORMAL;
static bool         pendingFlash;
static CRGB         pendingColor;
static int          pendingCount;

// LED task private — only accessed from the LED task.
static CRGB     leds[NUM_LEDS];
static LEDState localState     = LED_STATE_NORMAL;
static uint32_t lastBreatheMs  = 0;
static bool     flashActive    = false;
static CRGB     flashColor;
static int      flashRemaining;
static bool     flashPhaseOn;
static uint32_t flashPhaseStart;

static void stepBreathe(CRGB base) {
    uint32_t now = millis();
    if (now - lastBreatheMs < BREATHE_FPS_MS) return;
    lastBreatheMs = now;
    float t    = (float)(now % BREATHE_PERIOD_MS) / BREATHE_PERIOD_MS;
    float wave = (sinf(t * 2.0f * (float)M_PI - (float)M_PI_2) + 1.0f) * 0.5f;
    CRGB c     = base;
    c.nscale8((uint8_t)(10 + wave * 200.0f));
    fill_solid(leds, NUM_LEDS, c);
    FastLED.show();
}

static void stepFlash() {
    uint32_t now     = millis();
    uint32_t elapsed = now - flashPhaseStart;
    if (flashPhaseOn) {
        if (elapsed >= FLASH_ON_MS) {
            flashPhaseOn    = false;
            flashPhaseStart = now;
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        }
    } else {
        if (elapsed >= FLASH_OFF_MS) {
            if (--flashRemaining <= 0) {
                flashActive = false;
                fill_solid(leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
            } else {
                flashPhaseOn    = true;
                flashPhaseStart = now;
                fill_solid(leds, NUM_LEDS, flashColor);
                FastLED.show();
            }
        }
    }
}

static void update() {
    // Snapshot shared state under spinlock — released before any FastLED work.
    taskENTER_CRITICAL(&mux);
    LEDState newState = sharedState;
    bool     hasPend  = pendingFlash;
    CRGB     pColor   = pendingColor;
    int      pCount   = pendingCount;
    if (hasPend) pendingFlash = false;
    taskEXIT_CRITICAL(&mux);

    // Apply state change.
    if (newState != localState) {
        localState  = newState;
        flashActive = false;
        if (newState == LED_STATE_NORMAL) {
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            FastLED.show();
        }
    }

    // Arm incoming flash (overwrites any in-progress flash).
    if (hasPend && localState == LED_STATE_NORMAL) {
        flashColor      = pColor;
        flashRemaining  = pCount;
        flashPhaseOn    = true;
        flashPhaseStart = millis();
        flashActive     = true;
        fill_solid(leds, NUM_LEDS, flashColor);
        FastLED.show();
    }

    if      (localState == LED_STATE_WIFI_LOST)   stepBreathe(ORANGE);
    else if (localState == LED_STATE_WIFI_PORTAL) stepBreathe(BLUE);
    else if (flashActive)                         stepFlash();
}

static void ledsTask(void*) {
    for (;;) {
        update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void ledsBegin() {
    FastLED.addLeds<LED_TYPE, PIN_LEDS, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(LED_MAX_BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    // Pin to core 1 (same core as loop()) so WS2811 interrupt-disable windows
    // don't interfere with the WiFi stack on core 0.
    xTaskCreatePinnedToCore(ledsTask, "leds", 2048, nullptr, 2, nullptr, 1);
}

void ledsSetState(LEDState state) {
    taskENTER_CRITICAL(&mux);
    sharedState = state;
    taskEXIT_CRITICAL(&mux);
}

static void startFlash(CRGB color, int count) {
    taskENTER_CRITICAL(&mux);
    if (sharedState == LED_STATE_NORMAL) {
        pendingFlash = true;
        pendingColor = color;
        pendingCount = count;
    }
    taskEXIT_CRITICAL(&mux);
}

void ledsFlashBadge()         { startFlash(GREEN,  2); }
void ledsFlashTrack()         { startFlash(BLUE,   1); }
void ledsFlashPlay()          { startFlash(PURPLE, 1); }
void ledsFlashStop()          { startFlash(RED,    1); }
void ledsFlashMopidyRestart() { startFlash(RED,    4); }
void ledsFlashError()         { startFlash(ORANGE, 3); }

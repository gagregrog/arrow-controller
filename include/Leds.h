#pragma once

enum LEDState {
    LED_STATE_NORMAL,
    LED_STATE_WIFI_LOST,
    LED_STATE_WIFI_PORTAL,
};

void ledsBegin();
void ledsSetState(LEDState state);

void ledsFlashBadge();
void ledsFlashTrack();
void ledsFlashPlay();
void ledsFlashStop();
void ledsFlashMopidyRestart();
void ledsFlashError();

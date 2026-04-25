#pragma once
#include <Arduino.h>

int    arrowQuickPlay(int index);
int    arrowPlay();
int    arrowStop();
int    arrowShuffle();
int    arrowNextTrack();
int    arrowPreviousTrack();
int    arrowRestartTrack();
int    arrowRestartMopidy();
String arrowGetQuickplay(); // returns JSON body or empty string on failure

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
String arrowGetQuickplay();
String arrowGetArtists();
String arrowGetArtistAlbums(const String& encodedArtist); // artist name must be percent-encoded
int    arrowPutQuickplay(int index, const String& jsonBody);
int    arrowClearQuickplay();

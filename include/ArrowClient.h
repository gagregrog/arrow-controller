#pragma once
#include <Arduino.h>

// Tri-state stereo power status reported by the piserver photoresistor sensor.
enum StereoStatus {
    STEREO_UNKNOWN = -1, // sensor unavailable, or the query failed
    STEREO_OFF     = 0,
    STEREO_ON      = 1,
};

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
String arrowGetIR();
int    arrowSendIR(const String& function, int count = 1);
String arrowGetStereo();          // raw JSON: {"on":bool|null,"sensor_enabled":bool}
StereoStatus arrowStereoStatus(); // parsed tri-state from the sensor reading
String arrowGetStereoConfig();               // GET /stereo/config -> config JSON
int    arrowPutStereoConfig(const String& jsonBody); // PUT /stereo/config
String arrowStereoSample(int count);         // POST /stereo/sample -> stats JSON ("" on failure)
int    arrowRebootPi();                       // POST /system/reboot
int    arrowShutdownPi();                     // POST /system/shutdown

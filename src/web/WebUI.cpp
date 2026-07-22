#include "web/WebUI.h"
#include "web/API.h"
#include "web/web_ui_html.h"
#include "ArrowClient.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

static String _putQuickplayBody;
static String _putStereoConfigBody;

static String urlEncode(const String& s) {
    String out;
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += c;
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", (uint8_t)c);
            out += buf;
        }
    }
    return out;
}

void webUIBegin() {
    apiGetServer()->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(
            200, "text/html", HTML_GZ, HTML_GZ_LEN);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    });

    apiGetServer()->on("/ip", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "application/json",
            "{\"ip\":\"" + WiFi.localIP().toString() + "\"}");
    });

    apiGetServer()->on("/api/quickplay", HTTP_GET, [](AsyncWebServerRequest* request) {
        String body = arrowGetQuickplay();
        if (body.isEmpty()) {
            request->send(502, "application/json", "{\"error\":\"upstream unavailable\"}");
        } else {
            request->send(200, "application/json", body);
        }
    });

    apiGetServer()->on("/api/quickplay", HTTP_DELETE, [](AsyncWebServerRequest* request) {
        int code = arrowClearQuickplay();
        if (code >= 200 && code < 300) {
            request->send(200, "application/json", "{\"ok\":true}");
        } else {
            request->send(502, "application/json", "{\"error\":\"upstream error\"}");
        }
    });

    apiAddBodyHandler([](AsyncWebServerRequest* req, uint8_t* data, size_t len,
                         size_t index, size_t total) {
        if (req->method() == HTTP_PUT && req->url().startsWith("/api/quickplay/")) {
            if (index == 0) _putQuickplayBody.clear();
            _putQuickplayBody += String((char*)data, len);
        } else if (req->method() == HTTP_PUT && req->url() == "/api/stereo/config") {
            if (index == 0) _putStereoConfigBody.clear();
            _putStereoConfigBody += String((char*)data, len);
        }
    });

    apiGetServer()->on("/api/ir", HTTP_GET, [](AsyncWebServerRequest* request) {
        String body = arrowGetIR();
        if (body.isEmpty()) {
            request->send(502, "application/json", "{\"error\":\"upstream unavailable\"}");
        } else {
            request->send(200, "application/json", body);
        }
    });

    // Sensor configuration (GET current values for the settings form).
    apiGetServer()->on("/api/stereo/config", HTTP_GET, [](AsyncWebServerRequest* request) {
        String body = arrowGetStereoConfig();
        if (body.isEmpty()) {
            request->send(502, "application/json", "{\"error\":\"upstream unavailable\"}");
        } else {
            request->send(200, "application/json", body);
        }
    });

    // Note: PUT /api/stereo/config is handled in the notFound handler below, not
    // as a fixed route. ESPAsyncWebServer only feeds request bodies to the global
    // body handler when the request falls through to the catch-all path; a fixed
    // .on() route would receive an empty body (the Pi would then 422).

    // Run one sensor sample burst and return the stats JSON for tuning.
    apiGetServer()->on("/api/stereo/sample", HTTP_POST, [](AsyncWebServerRequest* request) {
        int count = 100;
        if (request->hasParam("count")) {
            count = request->getParam("count")->value().toInt();
            if (count < 1) count = 1;
        }
        String body = arrowStereoSample(count);
        if (body.isEmpty()) {
            request->send(502, "application/json", "{\"error\":\"sensor unavailable\"}");
        } else {
            request->send(200, "application/json", body);
        }
    });

    // Drive the receiver volume to zero via the Pi's volume policy.
    apiGetServer()->on("/api/volume/floor", HTTP_POST, [](AsyncWebServerRequest* request) {
        int code = arrowFloorVolume();
        if (code >= 200 && code < 300) {
            request->send(200, "application/json", "{\"ok\":true}");
        } else {
            request->send(502, "application/json", "{\"error\":\"upstream error\"}");
        }
    });

    // Drive the receiver to the configured target volume via the Pi.
    apiGetServer()->on("/api/volume/startup", HTTP_POST, [](AsyncWebServerRequest* request) {
        int code = arrowStartupVolume();
        if (code >= 200 && code < 300) {
            request->send(200, "application/json", "{\"ok\":true}");
        } else {
            request->send(502, "application/json", "{\"error\":\"upstream error\"}");
        }
    });

    // Restart the mopidy music service.
    apiGetServer()->on("/api/service/mopidy/restart", HTTP_POST, [](AsyncWebServerRequest* request) {
        int code = arrowRestartMopidy();
        if (code >= 200 && code < 300) {
            request->send(200, "application/json", "{\"ok\":true}");
        } else {
            request->send(502, "application/json", "{\"error\":\"upstream error\"}");
        }
    });

    // Reboot / shut down the Raspberry Pi.
    apiGetServer()->on("/api/system/reboot", HTTP_POST, [](AsyncWebServerRequest* request) {
        int code = arrowRebootPi();
        if (code >= 200 && code < 300) {
            request->send(200, "application/json", "{\"ok\":true}");
        } else {
            request->send(502, "application/json", "{\"error\":\"upstream error\"}");
        }
    });

    apiGetServer()->on("/api/system/shutdown", HTTP_POST, [](AsyncWebServerRequest* request) {
        int code = arrowShutdownPi();
        if (code >= 200 && code < 300) {
            request->send(200, "application/json", "{\"ok\":true}");
        } else {
            request->send(502, "application/json", "{\"error\":\"upstream error\"}");
        }
    });

    apiAddNotFoundHandler([](AsyncWebServerRequest* req) -> bool {
        String url = req->url();

        // POST /api/ir/{function}[?count=N]
        if (req->method() == HTTP_POST && url.startsWith("/api/ir/")) {
            String function = url.substring(8);
            int count = 1;
            if (req->hasParam("count")) {
                count = req->getParam("count")->value().toInt();
                if (count < 1) count = 1;
            }
            int code = arrowSendIR(function, count);
            if (code >= 200 && code < 300) {
                req->send(200, "application/json", "{\"ok\":true}");
            } else {
                req->send(502, "application/json", "{\"error\":\"upstream error\"}");
            }
            return true;
        }

        // GET /api/stereo (exact) — proxy the Pi's stereo power status
        if (req->method() == HTTP_GET && url == "/api/stereo") {
            String body = arrowGetStereo();
            if (body.isEmpty()) {
                req->send(502, "application/json", "{\"error\":\"upstream unavailable\"}");
            } else {
                req->send(200, "application/json", body);
            }
            return true;
        }

        // GET /api/artists (exact)
        if (req->method() == HTTP_GET && url == "/api/artists") {
            String body = arrowGetArtists();
            if (body.isEmpty()) {
                req->send(502, "application/json", "{\"error\":\"upstream unavailable\"}");
            } else {
                req->send(200, "application/json", body);
            }
            return true;
        }

        // GET /api/artists/{name}/albums
        if (req->method() == HTTP_GET && url.startsWith("/api/artists/") && url.endsWith("/albums")) {
            String artist = url.substring(13, url.length() - 7); // strip prefix/suffix
            String body = arrowGetArtistAlbums(urlEncode(artist));
            if (body.isEmpty()) {
                req->send(502, "application/json", "{\"error\":\"upstream unavailable\"}");
            } else {
                req->send(200, "application/json", body);
            }
            return true;
        }

        // POST /api/quickplay/{index} — trigger playback
        if (req->method() == HTTP_POST && url.startsWith("/api/quickplay/")) {
            int idx = url.substring(15).toInt();
            int code = arrowQuickPlay(idx);
            if (code >= 200 && code < 300) {
                req->send(200, "application/json", "{\"ok\":true}");
            } else {
                req->send(502, "application/json", "{\"error\":\"upstream error\"}");
            }
            return true;
        }

        // PUT /api/stereo/config — body accumulated by the global body handler
        if (req->method() == HTTP_PUT && url == "/api/stereo/config") {
            int code = arrowPutStereoConfig(_putStereoConfigBody);
            if (code >= 200 && code < 300) {
                req->send(200, "application/json", "{\"ok\":true}");
            } else {
                req->send(502, "application/json", "{\"error\":\"upstream error\"}");
            }
            return true;
        }

        // PUT /api/quickplay/{index}
        if (req->method() == HTTP_PUT && url.startsWith("/api/quickplay/")) {
            int idx = url.substring(15).toInt();
            Serial.printf("[WebUI] PUT quickplay[%d] body: %s\n", idx, _putQuickplayBody.c_str());
            int code = arrowPutQuickplay(idx, _putQuickplayBody);
            Serial.printf("[WebUI] PUT quickplay[%d] Pi responded: %d\n", idx, code);
            if (code >= 200 && code < 300) {
                req->send(200, "application/json", "{\"ok\":true}");
            } else {
                req->send(502, "application/json", "{\"error\":\"upstream error\"}");
            }
            return true;
        }

        return false;
    });
}

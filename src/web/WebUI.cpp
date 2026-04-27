#include "web/WebUI.h"
#include "web/API.h"
#include "web/web_ui_html.h"
#include "ArrowClient.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

static String _putQuickplayBody;

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
        }
    });

    apiAddNotFoundHandler([](AsyncWebServerRequest* req) -> bool {
        String url = req->url();

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

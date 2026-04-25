#include "web/WebUI.h"
#include "web/API.h"
#include "web/web_ui_html.h"
#include "ArrowClient.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

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
}

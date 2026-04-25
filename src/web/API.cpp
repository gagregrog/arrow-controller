#include "web/API.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <vector>

struct StatusEntry { const char* key; StatusContributor fn; };

static AsyncWebServer* _server = nullptr;
static std::vector<NotFoundHandler> _notFoundHandlers;
static std::vector<BodyHandler> _bodyHandlers;
static std::vector<StatusEntry> _statusContributors;

void apiInit(uint16_t port) {
    _server = new AsyncWebServer(port);
}

void apiStart() {
    _server->on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        for (auto& entry : _statusContributors) {
            entry.fn(doc, entry.key);
        }
        String body;
        serializeJson(doc, body);
        request->send(200, "application/json", body);
    });

    _server->onNotFound([](AsyncWebServerRequest* request) {
        for (auto& h : _notFoundHandlers) {
            if (h(request)) return;
        }
        Serial.printf("[API] 404 %s\n", request->url().c_str());
        request->send(404, "text/plain", "Not Found");
    });

    _server->onRequestBody([](AsyncWebServerRequest* req, uint8_t* data, size_t len,
                              size_t index, size_t total) {
        for (auto& h : _bodyHandlers) {
            h(req, data, len, index, total);
        }
    });

    _server->begin();
    Serial.println("[API] Listening");
}

AsyncWebServer* apiGetServer() {
    return _server;
}

void apiAddNotFoundHandler(NotFoundHandler h) {
    _notFoundHandlers.push_back(h);
}

void apiAddBodyHandler(BodyHandler h) {
    _bodyHandlers.push_back(h);
}

void apiAddStatusContributor(const char* key, StatusContributor fn) {
    _statusContributors.push_back({key, fn});
}

#pragma once
#include <functional>
#include <ArduinoJson.h>

// Central API hub. ESPAsyncWebServer supports only one onNotFound and one
// onRequestBody per server instance, so this module owns both singletons and
// fans them out to per-module handlers registered via apiAddNotFoundHandler /
// apiAddBodyHandler. Each API module registers its own handlers independently
// — no module needs to know about any other.

class AsyncWebServer;
class AsyncWebServerRequest;

// Returns true if the handler sent a response; false to try the next handler.
using NotFoundHandler =
    std::function<bool(AsyncWebServerRequest*)>;
// Called per body chunk; handler must filter by method/URL internally.
using BodyHandler =
    std::function<void(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t)>;
// Fills a named section of the GET /status response.
using StatusContributor = std::function<void(JsonDocument& doc, const char* key)>;

void            apiInit(uint16_t port = 80);
void            apiStart();
AsyncWebServer* apiGetServer();
void            apiAddNotFoundHandler(NotFoundHandler h);
void            apiAddBodyHandler(BodyHandler h);
void            apiAddStatusContributor(const char* key, StatusContributor fn);

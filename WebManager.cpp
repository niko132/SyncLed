#include "WebManager.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

#include "PaletteManager.h"
#include "VirtualDeviceManager.h"
#include "PresetManager.h"
#include "ConnectivityManager.h"
#include "StorageManager.h"
#include "Alexa.h"
#include "WifiManager.h"

#include "Profiler.h"

using namespace std::placeholders;

ESPWebManager::ESPWebManager() : _server(80), _ws("/ws"), _wifiConfigWs("/ws/wifi_config") {

}

void ESPWebManager::begin() {
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    // respond to GET requests on URL /heap
    _server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    // send a file when /index is requested
    _server.on("/index", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html");
    });
    _server.on("/index.js", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.js");
    });
    _server.on("/index.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.css");
    });

    _server.on("/grapick.min.js", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/grapick.min.js");
    });
    _server.on("/grapick.min.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/grapick.min.css");
    });



    _server.on("/wifi_config", HTTP_ANY, [](AsyncWebServerRequest *request) {
        // request->send(LittleFS, "/wifi_config.html");
        request->send(LittleFS, "/wifi_config.html");
    });

    _server.on("/wifi_config.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/wifi_config.css");
    });

    _server.onNotFound([](AsyncWebServerRequest *request) {
        // TODO: handle alexa
        // TODO: only redirect when in AP mode
        request->redirect("http://" + WiFi.softAPIP().toString() + "/wifi_config");
    });



    _ws.onEvent(std::bind(&ESPWebManager::onWSEvent, this, _1, _2, _3, _4, _5, _6));
    _server.addHandler(&_ws);

    _wifiConfigWs.onEvent(std::bind(&ESPWifiManager::onWSEvent, WifiManager, _1, _2, _3, _4, _5, _6));
    _server.addHandler(&_wifiConfigWs);

    // Alexa.h starts the server
    // _server.begin();

    _server.begin();
}

AsyncWebServer* ESPWebManager::getServer() {
    return &_server;
}


void ESPWebManager::onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT) {
        DynamicJsonDocument doc(5120);
        JsonObject jsonObj = doc.to<JsonObject>();

        PaletteManager.toJson(jsonObj);
        VirtualDeviceManager.toJson(jsonObj);
        PresetManager.toJson(jsonObj);
        ConnectivityManager.toJson(jsonObj);
        Alexa.toJson(jsonObj);

        String json;
        serializeJson(doc, json);

        client->text(json);
    } else if(type == WS_EVT_DISCONNECT) {
        Serial.println("Disconnect");
    } else if(type == WS_EVT_ERROR) {
        Serial.println("Error");
    } else if(type == WS_EVT_PONG) {
        Serial.println("Pong");
    } else if(type == WS_EVT_DATA) {
        Profiler.addTimestamp();

        DynamicJsonDocument doc(5120);
        DeserializationError err = deserializeJson(doc, data, len);

        if (err) {
            Serial.print("Deserialization failed: ");
            Serial.println(err.c_str());
        } else {
            JsonObject jsonObj = doc.as<JsonObject>();

            Profiler.addTimestamp();
            PaletteManager.fromJson(jsonObj);
            Profiler.addTimestamp();
            VirtualDeviceManager.fromJson(jsonObj);
            Profiler.addTimestamp();
            PresetManager.fromJson(jsonObj);
            Profiler.addTimestamp();
            Alexa.fromJson(jsonObj);
            Profiler.addTimestamp();

            StorageManager.storeConfig();
            Profiler.addTimestamp();
        }

        Profiler.print();
    }
}

void ESPWebManager::sendToAllClients(String text) {
    _ws.textAll(text);
}

void ESPWebManager::sendToAllWifiConfigClients(String text) {
    _wifiConfigWs.textAll(text);
}

ESPWebManager WebManager;

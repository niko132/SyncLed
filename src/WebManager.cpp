#include "WebManager.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

#include "PaletteManager.h"
#include "VirtualDeviceManager.h"
#include "OverlayManager.h"
#include "PresetManager.h"
#include "ConnectivityManager.h"
#include "StorageManager.h"
#include "Alexa.h"
#include "WifiManager.h"

#include "Profiler.h"

using namespace std::placeholders;

ESPWebManager::ESPWebManager() : _server(80), _ws("/ws"), _wifiConfigWs("/ws/wifi_config"), _liveDataWs("/ws/live") {

}

void ESPWebManager::begin() {
    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    // respond to GET requests on URL /heap
    _server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    // serve all contents of the www directory
    _server.serveStatic("/", LittleFS, "/www/").setCacheControl("max-age=604800");
    _server.rewrite("/", "/index.html").setFilter(ON_STA_FILTER);
    _server.rewrite("/", "/wifi_config.html").setFilter(ON_AP_FILTER);

    _server.on("/upload_gif.html", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
    }, [=](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index) {
            Serial.println("Upload started");

            // String path = "/gif/" + filename;
            String path = "/gif/test2.gif";
            _currentFile = LittleFS.open(path, "w");
            if (!_currentFile) {
                Serial.println("Failed to open file: " + path);
            }
        }

        if (!_currentFile) {
            return;
        }

        Serial.print("Uploading: ");
        Serial.print(index);
        Serial.print("...");
        Serial.println((index + len));
        _currentFile.write(data, len);

        if (final) {
            Serial.println("Upload finished");
            _currentFile.close();
        }
    });

    // not found needs to be handled as the last one
    _server.onNotFound([](AsyncWebServerRequest *request) {
        // TODO: handle alexa
        // TODO: only redirect when in AP mode
        String redirectUrl = "";
        if (ON_STA_FILTER(request)) {
            redirectUrl = "http://" + WiFi.localIP().toString() + "/";
        } else {
            redirectUrl = "http://" + WiFi.softAPIP().toString() + "/";
        }
        request->redirect(redirectUrl);
    });

    // add all websocket handlers
    _ws.onEvent(std::bind(&ESPWebManager::onWSEvent, this, _1, _2, _3, _4, _5, _6));
    _server.addHandler(&_ws);

    _wifiConfigWs.onEvent(std::bind(&ESPWifiManager::onWSEvent, WifiManager, _1, _2, _3, _4, _5, _6));
    _server.addHandler(&_wifiConfigWs);

    _server.addHandler(&_liveDataWs);

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
        OverlayManager.toJson(jsonObj);
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
            OverlayManager.fromJson(jsonObj);
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


void ESPWebManager::sendLiveData(uint8_t *buf, unsigned long len) {
    _liveDataWs.binaryAll(buf, len);
}

ESPWebManager WebManager;

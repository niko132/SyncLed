#include "StorageManager.h"

#include <ArduinoJson.h>

#include "PaletteManager.h"
#include "VirtualDeviceManager.h"
#include "OverlayManager.h"
#include "PresetManager.h"
#include "Alexa.h"

void ESPStorageManager::begin() {

};

void ESPStorageManager::loadConfig() {
    DynamicJsonDocument doc(5120);
    if (!readJson("/config.json", doc)) {
        Serial.println("Loading Default Config...");
        DeserializationError err = deserializeJson(doc, DEFAULT_CONFIG);

        if (err) {
            Serial.print("Deserialization failed: ");
            Serial.println(err.c_str());
            doc.to<JsonObject>(); // just use an empty object
        } else {
            Serial.println("Deserialization complete");
        }
    }
    JsonObject jsonObj = doc.as<JsonObject>();

    PaletteManager.fromJson(jsonObj);
    VirtualDeviceManager.fromJson(jsonObj);
    OverlayManager.fromJson(jsonObj);
    PresetManager.fromJson(jsonObj);
    Alexa.fromJson(jsonObj);
};

void ESPStorageManager::storeConfig() {
    // store in the next loop cycle to reduce memory usage by ArduinoJson
    _storeFlag = true;
};

void ESPStorageManager::update() {
    if (_storeFlag) {
        DynamicJsonDocument doc(5120);
        JsonObject jsonObj = doc.to<JsonObject>();

        PaletteManager.toJson(jsonObj);
        VirtualDeviceManager.toJson(jsonObj);
        OverlayManager.toJson(jsonObj);
        PresetManager.toJson(jsonObj);
        Alexa.toJson(jsonObj);

        File f = LittleFS.open("/config.json", "w");
        // TODO: serialize to string
        // TODO: write string partially to disk (10ms intervals)
        serializeJson(jsonObj, f);
        f.close();

        _storeFlag = false;
    }
}

ESPStorageManager StorageManager;

#include "Alexa.h"

#include "WebManager.h"
#include "VirtualDeviceManager.h"
#include "PaletteManager.h"

using namespace std::placeholders;

void ESPAlexa::begin() {
    AsyncWebServer *server = WebManager.getServer();

    server->onNotFound([this](AsyncWebServerRequest *request) {
        if (!_alexa.handleAlexaApiCall(request)) {
            //whatever you want to do with 404s
            request->send(404, "text/plain", "Not found");
        }
    });

    _alexa.begin(server);
}

void ESPAlexa::update() {
    _alexa.loop();
}

void ESPAlexa::deviceCallback(EspalexaDevice* d) {
    EspalexaDeviceProperty prop = d->getLastChangedProperty();

    if (EspalexaDeviceProperty::xy == prop || EspalexaDeviceProperty::hs == prop || EspalexaDeviceProperty::ct == prop) {
        Serial.println("Alexa change color");

        Palette *palette = PaletteManager.getPaletteByName("AlexaPalette");
        if (palette == NULL) {
            palette = PaletteManager.createPalette("AlexaPalette");
        }

        palette->clear();
        palette->addColorKey(0.0, d->getRGB());
        unsigned long paletteId = palette->getId();

        for (vd_map_itr it = VirtualDeviceManager.vdBegin(); it != VirtualDeviceManager.vdEnd(); it++) {
            VirtualDevice *vd = it->second;
            vd->setPaletteId(paletteId);
        }
    } else if (EspalexaDeviceProperty::bri == prop) {
        Serial.println("Alexa change brightness");

        uint8_t brightness = d->getValue();
        for (vd_map_itr it = VirtualDeviceManager.vdBegin(); it != VirtualDeviceManager.vdEnd(); it++) {
            VirtualDevice *vd = it->second;
            vd->setBrightness(brightness);
        }
    } else {
        Serial.println("Alexa change on/off");

        bool on = d->getValue();
        VirtualDeviceManager.setOn(on);
    }
}


void ESPAlexa::registerDevice(EspalexaDevice *d) {
    _alexa.addDevice(d);
}


void ESPAlexa::fromJson(JsonObject &root) {
    _name = root["an"] | _name;

    if (_alexaDevice == NULL) {
        _alexaDevice = new EspalexaDevice(_name, std::bind(&ESPAlexa::deviceCallback, this, _1), EspalexaDeviceType::color);
        _alexa.addDevice(_alexaDevice);
        Serial.print("Alexa Device created: ");
        Serial.println(_name);
    } else {
        _alexaDevice->setName(_name);
        Serial.print("Alexa Device name changed: ");
        Serial.println(_name);
    }
}

void ESPAlexa::toJson(JsonObject &root) {
    root["an"] = _name;
}

ESPAlexa Alexa;

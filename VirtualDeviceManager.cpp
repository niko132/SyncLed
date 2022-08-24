#include "VirtualDeviceManager.h"

#include "OverlayManager.h"

#include "WebManager.h"
#include "TimeManager.h"

ESPVirtualDeviceManager::ESPVirtualDeviceManager() : _strip(LED_COUNT) {

}

void ESPVirtualDeviceManager::begin() {
    _strip.Begin();
    _strip.Show();

    _lastUpdateMillis = millis() - UPDATE_MILLIS;
}

void ESPVirtualDeviceManager::update() {
    unsigned long now = millis();
    unsigned long delta = now - _lastUpdateMillis;

    if (delta > UPDATE_MILLIS) {
        // initialize to black
        for (size_t i = 0; i < LED_COUNT; i++) {
            _colors[i * 3] = 0;
            _colors[i * 3 + 1] = 0;
            _colors[i * 3 + 2] = 0;
        }

        for (vd_map_itr it = _virtualDevices.begin(); it != _virtualDevices.end(); it++) {
            VirtualDevice *vd = it->second;
            vd->update(delta);
        }



        // maybe move this but we need it right after the update
        if (now - _lastLiveUpdateMillis > LIVE_UPDATE_MILLIS) {
            WebManager.sendLiveData(_colors, LED_COUNT * 3);
            _lastLiveUpdateMillis = now;
        }



        unsigned long fadeMillis = 1000;
        float briPerMilli = 255.0 / fadeMillis;

        if (_state == OFF) {
            _currentFadeBri = 0.0;
        } else if (_state == FADE_IN) {
            float briInc = briPerMilli * delta;
            _currentFadeBri += briInc;

            if (_currentFadeBri >= 255.0) {
                _currentFadeBri = 255.0;
                _state = ON;
            }
        } else if (_state == ON) {
            _currentFadeBri = 255.0;
        } else if (_state == FADE_OUT) {
            float briDec = briPerMilli * delta;
            _currentFadeBri -= briDec;

            if (_currentFadeBri <= 0.0) {
                _currentFadeBri = 0.0;
                _state = OFF;
            }
        }


        if (_useBrightnessControl) {
            float currentTime = TimeManager.getHours() + TimeManager.getMinutes() / 60.0f + TimeManager.getSeconds() / 3600.0f;
            float brightness = 1.0;

            if (_numBrightnessControlEntries == 0) {
                brightness = 1.0;
            } else if (_numBrightnessControlEntries == 1) {
                brightness = _brightnessControlEntries[1];
            } else {
                size_t nextIndex = 0;
                while(_brightnessControlEntries[nextIndex * 2] < currentTime) {
                    nextIndex++;
                }
                size_t prevIndex = (_numBrightnessControlEntries + nextIndex - 1) % _numBrightnessControlEntries;

                float frac = (currentTime - _brightnessControlEntries[prevIndex * 2]) / (_brightnessControlEntries[nextIndex * 2] - _brightnessControlEntries[prevIndex * 2]);
                brightness = (1.0 - frac) * _brightnessControlEntries[prevIndex * 2 + 1] + frac * _brightnessControlEntries[nextIndex * 2 + 1];
            }

            _currentFadeBri *= brightness;
        }


        for (size_t i = 0; i < LED_COUNT; i++) {
            _colors[i * 3] = (uint8_t)(_currentFadeBri / 255.0 * _colors[i * 3] + 0.5);
            _colors[i * 3 + 1] = (uint8_t)(_currentFadeBri / 255.0 * _colors[i * 3 + 1] + 0.5);
            _colors[i * 3 + 2] = (uint8_t)(_currentFadeBri / 255.0 * _colors[i * 3 + 2] + 0.5);
        }

        if (_useBrightnessCorrection) {
            for (size_t i = 0; i < LED_COUNT; i++) {
                _colors[i * 3] = BRI_LOOKUP[_colors[i * 3]];
                _colors[i * 3 + 1] = BRI_LOOKUP[_colors[i * 3 + 1]];
                _colors[i * 3 + 2] = BRI_LOOKUP[_colors[i * 3 + 2]];
            }
        }


        // apply the overlay right before we show the leds
        OverlayManager.update(_colors, LED_COUNT);


        for (size_t i = 0; i < LED_COUNT; i++) {
            RgbColor color(_colors[i*3], _colors[i*3+1], _colors[i*3+2]);
            _strip.SetPixelColor(i, color);
        }

        _strip.Show();
        _lastUpdateMillis = now;
    }
}

vd_map_itr ESPVirtualDeviceManager::vdBegin() {
    return _virtualDevices.begin();
}

vd_map_itr ESPVirtualDeviceManager::vdEnd() {
    return _virtualDevices.end();
}

void ESPVirtualDeviceManager::fadeIn() {
    _state = FADE_IN;
}

void ESPVirtualDeviceManager::fadeOut() {
    _state = FADE_OUT;
}

void ESPVirtualDeviceManager::setOn(bool on) {
    if (on) {
        fadeIn();
    } else {
        fadeOut();
    }
}

void ESPVirtualDeviceManager::fromJson(JsonObject &root) {
    JsonArray vdIds = root["vdIds"];
    JsonArray vds = root["vds"];

    if (vdIds) {
        std::vector<unsigned long> create;

        for (JsonVariant vdIdVar : vdIds) {
            unsigned long vdId = vdIdVar.as<unsigned long>();

            if (!_virtualDevices.count(vdId)) {
                Serial.print("Create ");
                create.push_back(vdId);
            }
        }

        for (vd_map_itr it = _virtualDevices.begin(); it != _virtualDevices.end(); ) {
            unsigned long id = it->first;

            bool found = false;

            for (JsonVariant vdIdVar : vdIds) {
                unsigned long vdId = vdIdVar.as<unsigned long>();
                if (id == vdId) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                // delete virtual device
                Serial.print("Delete ");
                Serial.println(id);
                VirtualDevice *vd = it->second;
                delete vd;
                _virtualDevices.erase(it++);
            } else {
                ++it;
            }
        }

        for (std::vector<unsigned long>::iterator it = create.begin(); it != create.end(); it++) {
            unsigned long id = *it;
            VirtualDevice *vd = new VirtualDevice(_colors, LED_COUNT);
            _virtualDevices[id] = vd;
        }
    }

    for (JsonVariant vdVar : vds) {
        JsonObject vdObj = vdVar.as<JsonObject>();
        unsigned long vdId = vdObj["id"];

        if (_virtualDevices.count(vdId)) {
            VirtualDevice *vd = _virtualDevices[vdId];
            vd->fromJson(vdObj);
        }
    }


    bool on = root["on"] | _state != OFF;
    if (on) {
        _state = FADE_IN;
    } else {
        _state = FADE_OUT;
    }


    _useBrightnessControl = root["briCtrl"] | _useBrightnessControl;
    JsonArray brightnessControls = root["briCtrls"];

    if (brightnessControls) {
        if (_brightnessControlEntries) {
            delete[] _brightnessControlEntries;
            _brightnessControlEntries = nullptr;
            _numBrightnessControlEntries = 0;
        }

        _numBrightnessControlEntries = brightnessControls.size();
        _brightnessControlEntries = new float[_numBrightnessControlEntries * 2];

        size_t counter = 0;
        for (JsonVariant controlEntryVar : brightnessControls) {
            JsonObject controlEntry = controlEntryVar.as<JsonObject>();
            float time = controlEntry["t"] | -1.0f;
            float brightness = controlEntry["b"] | -1.0f;
            _brightnessControlEntries[counter * 2] = time;
            _brightnessControlEntries[counter * 2 + 1] = brightness;
            counter++;
        }
    }
}

void ESPVirtualDeviceManager::toJson(JsonObject &root) {
    root["on"] = _state != OFF;

    JsonArray vIds = root.createNestedArray("vdIds");
    JsonArray vds = root.createNestedArray("vds");

    for (vd_map_itr it = _virtualDevices.begin(); it != _virtualDevices.end(); it++) {
        unsigned long id = it->first;
        VirtualDevice *vd = it->second;

        vIds.add(id);
        JsonObject vdObj = vds.createNestedObject();
        vdObj["id"] = id;
        vd->toJson(vdObj);
    }

    root["nLeds"] = LED_COUNT;
    root["briCtrl"] = _useBrightnessControl;

    JsonArray brightnessControls = root.createNestedArray("briCtrls");
    if (_brightnessControlEntries && _numBrightnessControlEntries) {
        for (size_t i = 0; i < _numBrightnessControlEntries; i++) {
            float time = _brightnessControlEntries[i * 2];
            float brightness = _brightnessControlEntries[i * 2 + 1];

            JsonObject controlEntry = brightnessControls.createNestedObject();
            controlEntry["t"] = time;
            controlEntry["b"] = brightness;
        }
    }
}

ESPVirtualDeviceManager VirtualDeviceManager;

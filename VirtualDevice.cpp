#include "VirtualDevice.h"

#include "TimeManager.h"
#include "EffectManager.h"
#include "PaletteManager.h"

#include <NeoPixelBus.h>

VirtualDevice::VirtualDevice(uint8_t *leds, size_t count) {
    _leds = leds;
    _maxIndex = count;

    _startIndex = 0;
    _endIndex = count;

    _effect = EffectManager.getEffectById(EFFECT_DOT);
    _paletteId = 3; // RedAndGreen

    _bri = 255;
    _mirror = false;
}

VirtualDevice::~VirtualDevice() {
    if (_effect) {
        delete _effect;
        _effect = NULL;
    }
}

void VirtualDevice::update(unsigned long delta) {
    Palette *palette = PaletteManager.getPaletteById(_paletteId);

    size_t endIndex = _endIndex < _maxIndex ? _endIndex : _maxIndex;

    if (_effect && palette) {
        _effect->update(&_leds[_startIndex * 3], endIndex - _startIndex, palette);
    }

    if (_mirror) {
        for (size_t i = 0; i < (endIndex - _startIndex) / 2; i++) {
            size_t i1 = _startIndex + i;
            size_t i2 = endIndex - i - 1;;
            uint8_t rgb[3];

            rgb[0] = _leds[i1 * 3];
            rgb[1] = _leds[i1 * 3 + 1];
            rgb[2] = _leds[i1 * 3 + 2];

            _leds[i1 * 3] = _leds[i2 * 3];
            _leds[i1 * 3 + 1] = _leds[i2 * 3 + 1];
            _leds[i1 * 3 + 2] = _leds[i2 * 3 + 2];

            _leds[i2 * 3] = rgb[0];
            _leds[i2 * 3 + 1] = rgb[1];
            _leds[i2 * 3 + 2] = rgb[2];
        }
    }

    // adjust the brightness
    for (size_t i = _startIndex; i < endIndex; i++) {
        _leds[i * 3] = (uint8_t)(_bri / 255.0 * _leds[i * 3] + 0.5);
        _leds[i * 3 + 1] = (uint8_t)(_bri / 255.0 * _leds[i * 3 + 1] + 0.5);
        _leds[i * 3 + 2] = (uint8_t)(_bri / 255.0 * _leds[i * 3 + 2] + 0.5);
    }
}

uint8_t VirtualDevice::getBrightness() {
    return _bri;
}

void VirtualDevice::setBrightness(uint8_t brightness) {
    _bri = brightness;
}

void VirtualDevice::setPaletteId(unsigned long id) {
    _paletteId = id;
}

void VirtualDevice::updateConfig(uint8_t *leds, size_t count) {
    _leds = leds;
    _maxIndex = count;
    _endIndex = count < _endIndex ? count : _endIndex;
}

void VirtualDevice::fromJson(JsonObject &root) {
    _name = root["n"] | _name;

    _startIndex = root["sI"] | _startIndex;
    _endIndex = root["eI"] | _endIndex;

    unsigned long effectId = root["eId"] | 0;
    if (effectId) {
        // getEffectById(effectId);
        Effect *effect = EffectManager.getEffectById(effectId);
        if (effect) {
            if (_effect)
                delete _effect;

            _effect = effect;
        }
    }

    JsonObject effectDataObj = root["eD"];
    if (effectDataObj && _effect) {
        _effect->fromJson(effectDataObj);
    }

    _paletteId = root["pId"] | _paletteId;
    _bri = root["bri"] | _bri;
    _mirror = root["m"] | _mirror;
}

void VirtualDevice::toJson(JsonObject &root) {
    root["n"] = _name;

    root["sI"] = _startIndex;
    root["eI"] = _endIndex;

    root["eId"] = _effect->getId();
    JsonObject effectDataObj = root.createNestedObject("eD");
    _effect->toJson(effectDataObj);

    root["pId"] = _paletteId;
    root["bri"] = _bri;
    root["m"] = _mirror;
}

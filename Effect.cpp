#include "Effect.h"

#include "TimeManager.h"


Effect::Effect(unsigned long id) {
    _id = id;
}

Effect::~Effect() {

}

unsigned long Effect::getId() {
    return _id;
}

CyclicEffect::CyclicEffect(unsigned long id) : Effect(id) {
    _duration = 10000; // 10 secs
    _startPos = 0.0;
    _endPos = 1.0;
}

void CyclicEffect::update(uint8_t *leds, size_t count, Palette *palette) {
    float timeVal = (float)(TimeManager.syncedMillis() % _duration) / _duration;

    for (size_t i = 0; i < count; i++) {
        float posVal = (float)i / count;

        posVal *= _endPos - _startPos;
        posVal += _startPos;

        RgbColor color = colorAt(posVal, timeVal, count, palette);
        leds[i * 3] = color.R;
        leds[i * 3 + 1] = color.G;
        leds[i * 3 + 2] = color.B;
    }
}

void CyclicEffect::fromJson(JsonObject &root) {
    Effect::fromJson(root);

    _duration = root["d"] | _duration;
    _startPos = root["sP"] | _startPos;
    _endPos = root["eP"] | _endPos;
}

void CyclicEffect::toJson(JsonObject &root) {
    Effect::toJson(root);

    root["d"] = _duration;
    root["sP"] = _startPos;
    root["eP"] = _endPos;
}



SimulationEffect::SimulationEffect(unsigned long id) : Effect(id) {
    _ledCount = 0;
    _startMillis = 0;
    _lastMillis = 0;
    _speed = 1.0;
}

SimulationEffect::~SimulationEffect() {

}

void SimulationEffect::update(uint8_t *leds, size_t count, Palette *palette) {
    unsigned long currentMillis = millis();

    if (_ledCount != count) { // config changed
        init(_ledCount, count);
        _ledCount = count;
        _startMillis = currentMillis;
    } else {
        unsigned long elapsedMillis = currentMillis - _lastMillis;
        elapsedMillis *= _speed; // speed up or slow down

        simulate(leds, count, palette, elapsedMillis);
    }

    _lastMillis = currentMillis;
}

void SimulationEffect::fromJson(JsonObject &root) {
    Effect::fromJson(root);

    _speed = root["s"] | _speed;
}

void SimulationEffect::toJson(JsonObject &root) {
    Effect::toJson(root);

    root["s"] = _speed;
}

#ifndef BLINK_H
#define BLINK_H

#include "../Effect.h"
#include "../EffectManager.h"

class Blink : public CyclicEffect {
    private:
        int _numColors = 2;

    public:
        Blink() : CyclicEffect(EFFECT_BLINK) {

        };

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            float val = (float)((int)(timeVal * _numColors)) / _numColors;
            return palette->getColorAtPosition(val);
        };

        void fromJson(JsonObject &root) {
            CyclicEffect::fromJson(root);
            _numColors = root["nC"] | _numColors;
        };

        void toJson(JsonObject &root) {
            CyclicEffect::toJson(root);
            root["nC"] = _numColors;
        };
};

#endif
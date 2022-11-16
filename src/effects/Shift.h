#ifndef SHIFT_H
#define SHIFT_H

#include "../Effect.h"
#include "../EffectManager.h"

class Shift : public CyclicEffect {
    private:
        int _numColors = 2;

    public:
        Shift() : CyclicEffect(EFFECT_SHIFT) {

        };

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            float val = posVal - timeVal;
            if (val < 0.0)
                val += 1.0;
            
            val = (float)((int)(val * _numColors)) / _numColors;
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
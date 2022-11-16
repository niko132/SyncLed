#ifndef WIPE_H
#define WIPE_H

#include "../Effect.h"
#include "../EffectManager.h"

class Wipe : public CyclicEffect {
    private:
        int _numColors = 2;

    public:
        Wipe() : CyclicEffect(EFFECT_WIPE) {

        };

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            float val1 = (float)((int)(timeVal * _numColors)) / _numColors;
            float val2 = (float)(((int)(timeVal * _numColors) + 1) % _numColors) / _numColors;
            
            float frac = timeVal * _numColors - (int)(timeVal * _numColors);

            if (posVal < frac) {
                return palette->getColorAtPosition(val2);
            } else {
                return palette->getColorAtPosition(val1);
            }
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
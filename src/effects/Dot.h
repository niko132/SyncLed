#ifndef DOT_H
#define DOT_H

#include "../Effect.h"
#include "../Utils.h"
#include "../EffectManager.h"

class Dot : public CyclicEffect {
    private:
        float _size;

    public:
        Dot() : CyclicEffect(EFFECT_DOT) {
            _size = 20.0;
        }

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            float frac = _size / count;
            float fracHalf = frac / 2.0;

            float val = posVal - timeVal;
            if (val < 0.0)
                val += 1.0;

            RgbColor color = palette->getColorAtPosition(posVal);
            uint8_t bri = 0;

            if (val < frac) {
                // is lit
                if (val >= 0 && val < fracHalf) {
                    bri = (uint8_t)(val / fracHalf * 255.0 + 0.5);
                } else if (val >= fracHalf && val < frac) {
                    bri = (uint8_t)((frac - val) / fracHalf * 255.0 + 0.5);
                }
            }

            return color.Dim(bri);
        }

        void fromJson(JsonObject &root) {
            CyclicEffect::fromJson(root);
            _size = root["s"] | _size;
        }

        void toJson(JsonObject &root) {
            CyclicEffect::toJson(root);
            root["s"] = _size;
        }
};

#endif

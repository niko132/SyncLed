#ifndef PING_PONG_H
#define PING_PONG_H

#include "../Effect.h"
#include "../Utils.h"
#include "../EffectManager.h"

class PingPong : public CyclicEffect {
    private:
        float _size;

    public:
        PingPong() : CyclicEffect(EFFECT_PING_PONG) {
            _size = 20.0;
        }

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            float frac = _size / count;
            float fracHalf = frac / 2.0;

            timeVal *= 2;
            if (timeVal > 1.0) {
                timeVal = 2.0 - timeVal;
            }
            timeVal *= 1.0 - frac; // scale the time to not let the dot 'overflow'

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

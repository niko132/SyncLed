#ifndef SLIDE_2D_H
#define SLIDE_2D_H

#include "../Effect.h"
#include "../Utils.h"
#include "../EffectManager.h"
#include "../TimeManager.h"

class Slide2D : public CyclicEffect2D {
    private:
        float _startAngle = 0.0;
        float _rotPerSec = 0.0;

    public:
        Slide2D() : CyclicEffect2D(EFFECT_SLIDE_2D) {

        }

        RgbColor colorAt(float xPos, float yPos, float timeVal, size_t count, Palette *palette) {
            float rots = wrapValue(TimeManager.syncedMillis() / 1000.0 * _rotPerSec);
            float angle = _startAngle / 180.0 * M_PI + rots * 2 * M_PI;

            float skalar = xPos * cos(angle) + yPos * sin(angle);
            skalar /= 100.0;
            
            float val = wrapValue(skalar - timeVal);
            return palette->getColorAtPosition(val);
        };

        void fromJson(JsonObject &root) {
            CyclicEffect2D::fromJson(root);
            _startAngle = root["sA"] | _startAngle;
            _rotPerSec = root["rPS"] | _rotPerSec;
        }

        void toJson(JsonObject &root) {
            CyclicEffect2D::toJson(root);
            root["sA"] = _startAngle;
            root["rPS"] = _rotPerSec;
        }
};

#endif

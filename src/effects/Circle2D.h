#ifndef CIRCLE_2D_H
#define CIRCLE_2D_H

#include "../Effect.h"
#include "../Utils.h"
#include "../EffectManager.h"

class Circle2D : public CyclicEffect2D {
    public:
        Circle2D() : CyclicEffect2D(EFFECT_CIRCLE_2D) {

        }

        RgbColor colorAt(float xPos, float yPos, float timeVal, size_t count, Palette *palette) {
            float angle = atan2f(yPos, xPos);
            angle = (angle + M_PI) / (2.0 * M_PI);

            float val = angle - timeVal;
            if (val < 0.0)
                val += 1.0;

            return palette->getColorAtPosition(val);
        };
};

#endif

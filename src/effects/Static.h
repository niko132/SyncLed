#ifndef STATIC_H
#define STATIC_H

#include "../../Effect.h"
#include "../../EffectManager.h"

class Static : public CyclicEffect {
    public:
        Static() : CyclicEffect(EFFECT_STATIC) {

        }

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            return palette->getColorAtPosition(posVal);
        };
};

#endif

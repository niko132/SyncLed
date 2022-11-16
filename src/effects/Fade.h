#ifndef FADE_H
#define FADE_H

#include "../Effect.h"
#include "../EffectManager.h"

class Fade : public CyclicEffect {
    public:
        Fade() : CyclicEffect(EFFECT_FADE) {

        }

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            return palette->getColorAtPosition(timeVal);
        };
};

#endif

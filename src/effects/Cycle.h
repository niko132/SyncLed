#ifndef CYCLE_H
#define CYCLE_H

#include "../Effect.h"
#include "../Utils.h"
#include "../EffectManager.h"

class Cycle : public CyclicEffect {
    public:
        Cycle() : CyclicEffect(EFFECT_CYCLE) {

        }

        RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) {
            float val = posVal - timeVal;
            if (val < 0.0)
                val += 1.0;

            return palette->getColorAtPosition(val);
        };
};

#endif

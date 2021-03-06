#ifndef EFFECT_H
#define EFFECT_H

#include <NeoPixelBus.h>

class Effect {
    public:
        virtual void updateData() = 0;
        virtual RgbColor update(double timeVal, double posVal) = 0;
};

#endif // EFFECT_H

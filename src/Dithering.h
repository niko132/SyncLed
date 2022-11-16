#ifndef DITHERING_H
#define DITHERING_H

#include <NeoPixelBus.h>

// TODO: convert to class
// TODO: use it

uint8_t _updatesPerFrame = 0;
uint8_t _updateCounter = 0;

void initDither(uint8_t updatesPerFrame) {
    _updatesPerFrame = updatesPerFrame;
};

void updateDither() {
    _updateCounter = ++_updateCounter % _updatesPerFrame;
};

uint8_t getOffset(uint8_t time, size_t updates, uint8_t base) {
    if (time == 0)
        return 0;

    if (time <= base) {
        return updates % (base / time) == 0 ? 1 : 0;
    } else {
        return 1 - getOffset(base - time, updates, base);
    }
};

RgbColor applyDither(HsbColor color) {
    RgbColor c = HsbColor(color.H, color.S, 1.0);

    float fracs[3];
    fracs[0] = c.R * color.B;
    fracs[1] = c.G * color.B;
    fracs[2] = c.B * color.B;

    uint8_t cBasis[3];
    cBasis[0] = (uint8_t)fracs[0];
    cBasis[1] = (uint8_t)fracs[1];
    cBasis[2] = (uint8_t)fracs[2];

    uint8_t cTime[3];
    cTime[0] = (fracs[0] - cBasis[0]) * _updatesPerFrame + 0.5;
    cTime[1] = (fracs[1] - cBasis[1]) * _updatesPerFrame + 0.5;
    cTime[2] = (fracs[2] - cBasis[2]) * _updatesPerFrame + 0.5;

    return RgbColor(cBasis[0] + getOffset(cTime[0], _updateCounter, _updatesPerFrame), cBasis[1] + getOffset(cTime[1], _updateCounter, _updatesPerFrame), cBasis[2] + getOffset(cTime[2], _updateCounter, _updatesPerFrame));
};

#endif

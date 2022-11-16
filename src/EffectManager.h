#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H

#include "Effect.h"

#define EFFECT_STATIC 1
#define EFFECT_FADE 2
#define EFFECT_CYCLE 3
#define EFFECT_DOT 4
#define EFFECT_PING_PONG 5
#define EFFECT_CIRCLE_2D 6
#define EFFECT_SLIDE_2D 7
#define EFFECT_BLINK 8
#define EFFECT_WIPE 9
#define EFFECT_SHIFT 10

#define EFFECT_FIREWORK 100

#define EFFECT_FIREWORK_2D 101
#define EFFECT_GIF_2D 102
#define EFFECT_RANDOM_DOTS 103
#define EFFECT_RANDOM_PATTERN 104
#define EFFECT_SPARKLE_FILL 105
#define EFFECT_SPARKLE 106

#define EFFECT_RECEIVE 1000
#define EFFECT_RECEIVE_2D 1001

class ESPEffectManager {
    public:
        Effect* getEffectById(unsigned long id);
};

extern ESPEffectManager EffectManager;

#endif

#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H

#include "Effect.h"

#define EFFECT_STATIC 1
#define EFFECT_FADE 2
#define EFFECT_CYCLE 3
#define EFFECT_DOT 4
#define EFFECT_PING_PONG 5

#define EFFECT_FIREWORK 6

class ESPEffectManager {
    public:
        Effect* getEffectById(unsigned long id);
};

extern ESPEffectManager EffectManager;

#endif

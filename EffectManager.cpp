#include "EffectManager.h"

#include "src/effects/Static.h"
#include "src/effects/Fade.h"
#include "src/effects/Cycle.h"
#include "src/effects/Dot.h"
#include "src/effects/PingPong.h"

#include "src/effects/Firework.h"

Effect* ESPEffectManager::getEffectById(unsigned long id) {
    Effect *effect = NULL;

    switch(id) {
        case EFFECT_STATIC:
            effect = new Static();
            break;
        case EFFECT_FADE:
            effect = new Fade();
            break;
        case EFFECT_CYCLE:
            effect = new Cycle();
            break;
        case EFFECT_DOT:
            effect = new Dot();
            break;
        case EFFECT_PING_PONG:
            effect = new PingPong();
            break;
        case EFFECT_FIREWORK:
            effect = new Firework();
            break;
    }

    return effect;
}

ESPEffectManager EffectManager;

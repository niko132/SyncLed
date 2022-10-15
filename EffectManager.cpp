#include "EffectManager.h"

#include "src/effects/Static.h"
#include "src/effects/Fade.h"
#include "src/effects/Cycle.h"
#include "src/effects/Dot.h"
#include "src/effects/PingPong.h"
#include "src/effects/Circle2D.h"
#include "src/effects/Slide2D.h"
#include "src/effects/Firework2D.h"
#include "src/effects/Gif2D.h"
#include "src/effects/Receive.h"
#include "src/effects/Receive2D.h"
#include "src/effects/RandomDots.h"
#include "src/effects/RandomPattern.h"
#include "src/effects/Blink.h"
#include "src/effects/Wipe.h"
#include "src/effects/Shift.h"
#include "src/effects/SparkleFill.h"
#include "src/effects/Sparkle.h"

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
        case EFFECT_CIRCLE_2D:
            effect = new Circle2D();
            break;
        case EFFECT_SLIDE_2D:
            effect = new Slide2D();
            break;
        case EFFECT_FIREWORK:
            effect = new Firework();
            break;
        case EFFECT_FIREWORK_2D:
            effect = new Firework2D();
            break;
        case EFFECT_GIF_2D:
            effect = new Gif2D();
            break;
        case EFFECT_RECEIVE:
            effect = new Receive();
            break;
        case EFFECT_RECEIVE_2D:
            effect = new Receive2D();
            break;
        case EFFECT_RANDOM_DOTS:
            effect = new RandomDots();
            break;
        case EFFECT_RANDOM_PATTERN:
            effect = new RandomPattern();
            break;
        case EFFECT_BLINK:
            effect = new Blink();
            break;
        case EFFECT_WIPE:
            effect = new Wipe();
            break;
        case EFFECT_SHIFT:
            effect = new Shift();
            break;
        case EFFECT_SPARKLE_FILL:
            effect = new SparkleFill();
            break;
        case EFFECT_SPARKLE:
            effect = new Sparkle();
            break;
    }

    return effect;
}

ESPEffectManager EffectManager;

#include "EffectManager.h"

#include "effects/Static.h"
#include "effects/Fade.h"
#include "effects/Cycle.h"
#include "effects/Dot.h"
#include "effects/PingPong.h"
#include "effects/Circle2D.h"
#include "effects/Slide2D.h"
#include "effects/Firework2D.h"
#include "effects/Gif2D.h"
#include "effects/Receive.h"
#include "effects/Receive2D.h"
#include "effects/RandomDots.h"
#include "effects/RandomPattern.h"
#include "effects/Blink.h"
#include "effects/Wipe.h"
#include "effects/Shift.h"
#include "effects/SparkleFill.h"
#include "effects/Sparkle.h"

#include "effects/Firework.h"

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

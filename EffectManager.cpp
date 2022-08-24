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
            Serial.println("Creating Effect: GIF 2D");
            Serial.print("Size: ");
            Serial.println(sizeof(Gif2D));
            effect = new Gif2D();
            break;
        case EFFECT_RECEIVE:
            effect = new Receive();
            break;
        case EFFECT_RECEIVE_2D:
            effect = new Receive2D();
            break;
    }

    return effect;
}

ESPEffectManager EffectManager;

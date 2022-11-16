#ifndef EFFECT_H
#define EFFECT_H

#include <NeoPixelBus.h>
#include <ArduinoJson.h>

#include "Palette.h"

class Effect {
    private:
        unsigned long _id;

    public:
        Effect(unsigned long id);
        virtual ~Effect();

        unsigned long getId();
        virtual void update(uint8_t *leds, size_t count, Palette *palette) = 0;
        virtual void fromJson(JsonObject &root) {};
        virtual void toJson(JsonObject &root) {};
};

class CyclicEffect : public Effect {
    private:
        unsigned long _duration;
        float _startPos;
        float _endPos;

    public:
        CyclicEffect(unsigned long id);

        void update(uint8_t *leds, size_t count, Palette *palette);
        virtual RgbColor colorAt(float posVal, float timeVal, size_t count, Palette *palette) = 0;
        virtual void fromJson(JsonObject &root);
        virtual void toJson(JsonObject &root);
};

class SimulationEffect : public Effect {
    private:
        size_t _ledCount;
        unsigned long _startMillis;
        unsigned long _lastMillis;
        float _speed;

    public:
        SimulationEffect(unsigned long id);
        virtual ~SimulationEffect();

        virtual void init(size_t oldLedCount, size_t newLedCount) = 0;
        void update(uint8_t *leds, size_t count, Palette *palette);
        virtual void simulate(uint8_t *leds, size_t count, Palette *palette, unsigned long dTime) = 0;
        virtual void fromJson(JsonObject &root);
        virtual void toJson(JsonObject &root);
};


class CyclicEffect2D : public Effect {
    private:
        unsigned long _duration;
        float _startPos;
        float _endPos;

    public:
        CyclicEffect2D(unsigned long id);

        void update(uint8_t *leds, size_t count, Palette *palette);
        virtual RgbColor colorAt(float xPos, float yPos, float timeVal, size_t count, Palette *palette) = 0;
        virtual void fromJson(JsonObject &root);
        virtual void toJson(JsonObject &root);
};

class SimulationEffect2D : public Effect {
    private:
        size_t _ledCount;
        unsigned long _startMillis;
        unsigned long _lastMillis;
        float _speed;

    public:
        SimulationEffect2D(unsigned long id);
        virtual ~SimulationEffect2D();

        virtual void init() = 0;
        void update(uint8_t *leds, size_t count, Palette *palette);
        virtual void simulate(Palette *palette, unsigned long dTime) = 0;
        virtual RgbColor colorAt(float xPos, float yPos, Palette *palette) = 0;
        virtual void fromJson(JsonObject &root);
        virtual void toJson(JsonObject &root);
};


#endif

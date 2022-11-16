#ifndef VIRTUAL_DEVICE_H
#define VIRTUAL_DEVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Effect.h"
#include "Palette.h"

class VirtualDevice {
    private:
        String _name;
        size_t _startIndex;
        size_t _endIndex;
        uint8_t *_leds;
        size_t _maxIndex;

        Effect *_effect;
        unsigned long _paletteId;

        uint8_t _bri;
        bool _mirror;

    public:
        VirtualDevice(uint8_t *leds, size_t count);
        ~VirtualDevice();
        void update(unsigned long delta);

        uint8_t getBrightness();
        void setBrightness(uint8_t brightness);

        void setPaletteId(unsigned long id);

        void updateConfig(uint8_t *leds, size_t count);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

#endif

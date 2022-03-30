#ifndef PALETTE_H
#define PALETTE_H

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <ArduinoJson.h>
#include <map>

typedef std::map<float, uint32_t> color_map;
typedef color_map::iterator color_map_itr;

class Palette {
    private:
        unsigned long _id;
        String _name;
        color_map _colors;

    public:
        Palette();
        Palette(String name);
        Palette(unsigned long, String name);

        unsigned long getId();
        String getName();

        void clear();
        void addColorKey(float pos, uint8_t r, uint8_t g, uint8_t b);
        void addColorKey(float pos, uint32_t color);
        RgbColor getColorAtPosition(float pos);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

#endif

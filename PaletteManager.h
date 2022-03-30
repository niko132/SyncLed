#ifndef PALETTE_MANAGER_H
#define PALETTE_MANAGER_H

#include <ArduinoJson.h>
#include <map>

#include "Palette.h"

typedef std::map<unsigned long, Palette*> palette_map;
typedef palette_map::iterator palette_map_itr;

class ESPPaletteManager {
    private:
        palette_map _palettes;

    public:
        ESPPaletteManager();

        Palette* getPaletteById(unsigned long id);
        Palette* getPaletteByName(String name);

        Palette* createPalette(String name);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

extern ESPPaletteManager PaletteManager;

#endif

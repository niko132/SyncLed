#include "PaletteManager.h"

ESPPaletteManager::ESPPaletteManager() {

}

Palette* ESPPaletteManager::getPaletteById(unsigned long id) {
    if (_palettes.count(id)) {
        return _palettes[id];
    }

    return NULL;
}

Palette* ESPPaletteManager::getPaletteByName(String name) {
    for (palette_map_itr it = _palettes.begin(); it != _palettes.end(); it++) {
        Palette *palette = it->second;

        if (name == palette->getName()) {
            return palette;
        }
    }

    return NULL;
}

Palette* ESPPaletteManager::createPalette(String name) {
    unsigned long id = random((long)0x7FFFFFFF);
    Palette *palette = new Palette(id, name);
    _palettes[id] = palette;
    return palette;
}

void ESPPaletteManager::fromJson(JsonObject &root) {
    JsonArray pIds = root["pIds"];
    JsonArray ps = root["ps"];

    if (pIds) {
        std::vector<unsigned long> create;

        for (JsonVariant pIdVar : pIds) {
            unsigned long pId = pIdVar.as<unsigned long>();

            if (!_palettes.count(pId)) {
                create.push_back(pId);
            }
        }

        for (palette_map_itr it = _palettes.begin(); it != _palettes.end(); ) {
            unsigned long id = it->first;

            bool found = false;

            for (JsonVariant pIdVar : pIds) {
                unsigned long pId = pIdVar.as<unsigned long>();
                if (id == pId) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                // delete palette
                Palette *palette = it->second;
                delete palette;
                _palettes.erase(it++);
            } else {
                ++it;
            }
        }

        for (std::vector<unsigned long>::iterator it = create.begin(); it != create.end(); it++) {
            unsigned long id = *it;
            Palette *palette = new Palette();
            _palettes[id] = palette;
        }
    }

    for (JsonVariant pVar : ps) {
        JsonObject pObj = pVar.as<JsonObject>();
        unsigned long pId = pObj["id"];

        if (_palettes.count(pId)) {
            Palette *palette = _palettes[pId];
            palette->fromJson(pObj);
        }
    }
}

void ESPPaletteManager::toJson(JsonObject &root) {
    JsonArray pIds = root.createNestedArray("pIds");
    JsonArray ps = root.createNestedArray("ps");

    for (palette_map_itr it = _palettes.begin(); it != _palettes.end(); it++) {
        unsigned long id = it->first;
        Palette *palette = it->second;

        pIds.add(id);
        JsonObject paletteObj = ps.createNestedObject();
        paletteObj["id"] = id;
        palette->toJson(paletteObj);
    }
}

ESPPaletteManager PaletteManager;

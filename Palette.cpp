#include "Palette.h"

#include "Utils.h"

Palette::Palette() {

}

Palette::Palette(String name) {
    _name = name;
}

Palette::Palette(unsigned long id, String name) {
    _id = id;
    _name = name;
}

unsigned long Palette::getId() {
    return _id;
}

String Palette::getName() {
    return _name;
}

void Palette::clear() {
    _colors.clear();
}

void Palette::addColorKey(float pos, uint8_t r, uint8_t g, uint8_t b) {
    uint32_t color = rgbToColor(r, g, b);
    addColorKey(pos, color);
}

void Palette::addColorKey(float pos, uint32_t color) {
    _colors[pos] = color;
}

RgbColor Palette::getColorAtPosition(float pos) {
    if (_colors.size() == 0)
        return RgbColor(0, 0, 0);
    else if (_colors.size() == 1)
        return colorToRgbColor(_colors.begin()->second);

    // at least 2 keys exist
    uint32_t firstColor = 0;
    float beforeDist = 2.0;
    for (color_map_itr it = _colors.begin(); it != _colors.end(); it++) {
        float keyPos = it->first;

        if (keyPos <= pos && pos - keyPos < beforeDist) {
            firstColor = it->second;
            beforeDist = pos - keyPos;
        } else if (keyPos <= pos + 1.0 && pos + 1.0 - keyPos < beforeDist) {
            firstColor = it->second;
            beforeDist = pos + 1.0 - keyPos;
        }
    }

    uint32_t secondColor = 0;
    float afterDist = 2.0;
    for (color_map_itr it = _colors.begin(); it != _colors.end(); it++) {
        float keyPos = it->first;

        if (keyPos > pos && keyPos - pos < afterDist) {
            secondColor = it->second;
            afterDist = keyPos - pos;
        } else if (keyPos + 1.0 > pos && keyPos + 1.0 - pos < afterDist) {
            secondColor = it->second;
            afterDist = keyPos + 1.0 - pos;
        }
    }

    float fac = beforeDist / (beforeDist + afterDist);

    RgbColor firstCol = colorToRgbColor(firstColor);
    RgbColor secondCol = colorToRgbColor(secondColor);

    RgbColor blended = RgbColor::LinearBlend(firstCol, secondCol, fac);

    /*
    HsbColor firstHsb(firstCol);
    HsbColor secondHsb(secondCol);

    HsbColor blended = HsbColor::LinearBlend<NeoHueBlendShortestDistance>(firstHsb, secondHsb, fac);
    */

    return blended;
}

void Palette::fromJson(JsonObject &root) {
    _id = root["id"] | _id;
    _name = root["n"] | _name;

    JsonArray keysArray = root["ks"];
    if (keysArray) {
        _colors.clear();

        for (JsonVariant keyVar : keysArray) {
            JsonObject keyObj = keyVar.as<JsonObject>();

            float pos = keyObj["p"] | 0.0;
            uint32_t color = keyObj["c"] | 0;

            uint8_t r = colorToRed(color);
            uint8_t g = colorToGreen(color);
            uint8_t b = colorToBlue(color);

            addColorKey(pos, r, g, b);
        }
    }
}

void Palette::toJson(JsonObject &root) {
    root["id"] = _id;
    root["n"] = _name;
    JsonArray keysArray = root.createNestedArray("ks");

    for (color_map_itr it = _colors.begin(); it != _colors.end(); it++) {
        float pos = it->first;
        uint32_t color = it->second;

        JsonObject keyObj = keysArray.createNestedObject();
        keyObj["p"] = pos;
        keyObj["c"] = color;
    }
}

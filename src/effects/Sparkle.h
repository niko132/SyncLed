#ifndef SPARKLE_H
#define SPARKLE_H

#include "../../Effect.h"
#include "../../EffectManager.h"
#include "../../Utils.h"

class Sparkle : public SimulationEffect {
    private:
        unsigned long _lifetime;
        unsigned long _duration = 500;
        size_t _position = 0;
        size_t _size = 5;
        uint32_t _color = 0;

    public:
        Sparkle() : SimulationEffect(EFFECT_SPARKLE) {

        };

        void init(size_t oldLedCount, size_t newLedCount) {
            if (_position >= newLedCount) {
                _position = random(newLedCount - _size + 1);
            }
        };

        void simulate(uint8_t *leds, size_t count, Palette *palette, unsigned long dTime) {
            _lifetime += dTime;
            if (_lifetime > _duration) {
                _lifetime -= (int)((double)_lifetime / _duration) * _duration;

                _position = random(count - _size + 1);
                RgbColor c = palette->getColorAtPosition(random(100) / 100.0f);
                _color = rgbToColor(c.R, c.G, c.B);
            }

            for (size_t i = 0; i < count; i++) {
                if (i >= _position && i < _position + _size) {
                    leds[i * 3] = colorToRed(_color);
                    leds[i * 3 + 1] = colorToGreen(_color);
                    leds[i * 3 + 2] = colorToBlue(_color);
                } else {
                    leds[i * 3] = 0;
                    leds[i * 3 + 1] = 0;
                    leds[i * 3 + 2] = 0;
                }
            }
        };
        
        void fromJson(JsonObject &root) {
            SimulationEffect::fromJson(root);
            _size = root["si"] | _size;
        };
        
        void toJson(JsonObject &root) {
            SimulationEffect::toJson(root);
            root["si"] = _size;
        };
};

#endif

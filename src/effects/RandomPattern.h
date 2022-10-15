#ifndef RANDOM_PATTERN_H
#define RANDOM_PATTERN_H

#include "../../Effect.h"
#include "../../EffectManager.h"

struct rp_dot_list_t {
    uint32_t colorFrom;
    uint32_t colorTo;
    uint32_t colorCurr;
    size_t pos;
    int size;
    unsigned long lifetime;
    unsigned long maxLifetime;

    bool isFirst;
    struct rp_dot_list_t *next;
};

class RandomPattern : public SimulationEffect {
    private:
        int _size = 3;
        float _density = 0.5;
        int _offset = 0;

        rp_dot_list_t _dots;

    public:
        RandomPattern() : SimulationEffect(EFFECT_RANDOM_PATTERN) {
            _dots.isFirst = true;
            _dots.next = NULL;
        }

        ~RandomPattern() {

        }

        void init(size_t oldLedCount, size_t newLedCount) {
            if (newLedCount < oldLedCount) {
                for (rp_dot_list_t *it = &_dots; it->next != NULL; ) {
                    rp_dot_list_t *curr = it->next;
                    
                    if (curr->pos >= newLedCount) {
                        it->next = curr->next;
                        delete curr;
                        curr = NULL;
                    } else {
                        it = it->next;
                    }
                }
            }
        }

        void simulate(uint8_t *leds, size_t count, Palette *palette, unsigned long dTime) {
            size_t dotSum = 0;
            for (rp_dot_list_t *it = _dots.next; it != NULL; it = it->next) {
                dotSum += it->size;
            }

            float density = (float)dotSum / count;
            bool shouldSpawn = density < _density;

            if (shouldSpawn) {
                bool *occupied = new bool[count];
                for (size_t i = 0; i < count; i++) {
                    occupied[i] = false;
                }

                for (rp_dot_list_t *it = _dots.next; it != NULL; it = it->next) {
                    for (int i = 0; i < it->size && it->pos + i < count; i++) {
                        occupied[it->pos + i] = true;
                    }
                }

                std::vector<size_t> freePos;
                for (size_t i = _offset; i < count; i += (int)(_size / _density)) {
                    bool canSpawn = true;
                    for (int j = 0; j < _size; j++) {
                        if (occupied[i + j] || i + j >= count) {
                            canSpawn = false;
                            break;
                        }
                    }

                    if (canSpawn) {
                        freePos.push_back(i);
                    }
                }

                if (!freePos.empty()) {
                    size_t randPickIdx = millis() % freePos.size();
                    size_t pos = freePos[randPickIdx];

                    RgbColor color1 = palette->getColorAtPosition(random(100) / 100.0f);
                    RgbColor color2 = palette->getColorAtPosition(random(100) / 100.0f);

                    // spawn new dot at pos
                    rp_dot_list_t *newDot = new rp_dot_list_t;
                    newDot->colorFrom = rgbToColor(color1.R, color1.G, color1.B);
                    newDot->colorTo = rgbToColor(color2.R, color2.G, color2.B);
                    newDot->colorCurr = newDot->colorFrom;
                    newDot->pos = pos;
                    newDot->size = _size;
                    newDot->lifetime = 0;
                    newDot->maxLifetime = (millis() % 16) * 1000 + 5000;
                    newDot->isFirst = false;
                    newDot->next = _dots.next;
                    _dots.next = newDot;
                } else {
                    // no spawn position found
                }

                delete[] occupied;
            }

            // update
            for (rp_dot_list_t *it = &_dots; it->next != NULL; ) {
                rp_dot_list_t *curr = it->next;

                curr->lifetime += dTime;

                RgbColor color = RgbColor::LinearBlend(colorToRgbColor(curr->colorFrom), colorToRgbColor(curr->colorTo), (float)curr->lifetime / curr->maxLifetime);
                curr->colorCurr = rgbToColor(color.R, color.G, color.B);

                if (curr->lifetime >= curr->maxLifetime) {
                    if (curr->size != _size || (curr->pos - _offset) % (int)((_size / _density)) != 0) {
                        // delete
                        it->next = curr->next;
                        delete curr;
                        curr = NULL;
                    } else {
                        // new color
                        RgbColor newColor = palette->getColorAtPosition(random(100) / 100.0f);

                        curr->colorFrom = curr->colorTo;
                        curr->colorTo = rgbToColor(newColor.R, newColor.G, newColor.B);
                        curr->colorCurr = curr->colorFrom;
                        curr->lifetime = 0;
                        curr->maxLifetime = (millis() % 16) * 1000 + 5000;

                        it = it->next;
                    }
                } else {
                    it = it->next;
                }
            }

            // render
            for (rp_dot_list_t *it = _dots.next; it != NULL; it = it->next) {
                RgbColor rgb = colorToRgbColor(it->colorCurr);

                for (int i = 0; i < it->size; i++) {
                    leds[(it->pos + i) * 3] = rgb.R;
                    leds[(it->pos + i) * 3 + 1] = rgb.G;
                    leds[(it->pos + i) * 3 + 2] = rgb.B;
                }
            }
        };

        
        void fromJson(JsonObject &root) {
            SimulationEffect::fromJson(root);
            _size = root["si"] | _size;
            _density = root["d"] | _density;
            _offset = root["o"] | _offset;
        };
        
        void toJson(JsonObject &root) {
            SimulationEffect::toJson(root);
            root["si"] = _size;
            root["d"] = _density;
            root["o"] = _offset;
        };
};

#endif
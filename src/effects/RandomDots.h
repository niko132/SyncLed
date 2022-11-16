#ifndef RANDOM_DOTS_H
#define RANDOM_DOTS_H

#include "../Effect.h"
#include "../EffectManager.h"

struct rd_dot_list_t {
    uint32_t color;
    size_t pos;
    int size;
    float bri;
    unsigned long lifetime;
    unsigned long maxLifetime;

    bool isFirst;
    struct rd_dot_list_t *next;
};

class RandomDots : public SimulationEffect {
    private:
        int _size = 3;
        float _density = 0.5;

        rd_dot_list_t _dots;

    public:
        RandomDots() : SimulationEffect(EFFECT_RANDOM_DOTS) {
            _dots.isFirst = true;
            _dots.next = NULL;
        }

        ~RandomDots() {

        }

        void init(size_t oldLedCount, size_t newLedCount) {
            if (newLedCount < oldLedCount) {
                for (rd_dot_list_t *it = &_dots; it->next != NULL; ) {
                    rd_dot_list_t *curr = it->next;
                    
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
            for (rd_dot_list_t *it = _dots.next; it != NULL; it = it->next) {
                dotSum += it->size;
            }

            float density = (float)dotSum / count;
            bool shouldSpawn = density < _density;

            if (shouldSpawn) {
                bool *occupied = new bool[count];
                for (size_t i = 0; i < count; i++) {
                    occupied[i] = false;
                }

                for (rd_dot_list_t *it = _dots.next; it != NULL; it = it->next) {
                    for (int i = 0; i < it->size && it->pos + i < count; i++) {
                        occupied[it->pos + i] = true;
                    }
                }

                std::vector<size_t> freePos;
                for (size_t i = 0; i < count; i++) {
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

                    RgbColor color = palette->getColorAtPosition((millis() % 100) / 100.0f);

                    // spawn new dot at pos
                    rd_dot_list_t *newDot = new rd_dot_list_t;
                    newDot->color = rgbToColor(color.R, color.G, color.B);
                    newDot->pos = pos;
                    newDot->size = _size;
                    newDot->bri = 0.0f;
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
            for (rd_dot_list_t *it = &_dots; it->next != NULL; ) {
                rd_dot_list_t *curr = it->next;

                curr->lifetime += dTime;
                if (curr->lifetime < curr->maxLifetime / 2.0) {
                    curr->bri = (float)curr->lifetime / curr->maxLifetime * 2.0f;
                } else {
                    curr->bri = (1.0f - (float)curr->lifetime / curr->maxLifetime) * 2.0f;
                }

                if (curr->lifetime >= curr->maxLifetime) {
                    // delete
                    it->next = curr->next;
                    delete curr;
                    curr = NULL;
                } else {
                    it = it->next;
                }
            }

            // render
            for (rd_dot_list_t *it = _dots.next; it != NULL; it = it->next) {
                RgbColor rgb = colorToRgbColor(it->color);
                rgb.R = (uint8_t)(rgb.R * it->bri);
                rgb.G = (uint8_t)(rgb.G * it->bri);
                rgb.B = (uint8_t)(rgb.B * it->bri);

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
        };
        
        void toJson(JsonObject &root) {
            SimulationEffect::toJson(root);
            root["si"] = _size;
            root["d"] = _density;
        };
};

#endif
#ifndef FIREWORK_H
#define FIREWORK_H

#include "../../Effect.h"
#include "../../Utils.h"
#include "../../EffectManager.h"

struct sparkle_t {
    uint32_t color;
    float bri;
    float pos;
    float vel;
};

struct firework_t {
    int state;
    sparkle_t sparkles[10];
};

class Firework : public SimulationEffect {
    private:
        firework_t *data;

    public:
        Firework() : SimulationEffect(EFFECT_FIREWORK) {
            data = NULL;
        };

        ~Firework() {
            if (data) {
                delete[] data;
                data = NULL;
            }
        }

        void init(size_t oldLedCount, size_t newLedCount) {
            if (data) {
                delete[] data;
                data = NULL;
            }

            data = new firework_t[3];
            for (size_t j = 0; j < 3; j++) {
                data[j].state = 0;
            }
        };

        void simulate(uint8_t *leds, size_t count, Palette *palette, unsigned long dTime) {
            float timeFac = dTime / 1000.0;

            for (size_t i = 0; i < count ; i++) {
                leds[i * 3] = 0;
                leds[i * 3 + 1] = 0;
                leds[i * 3 + 2] = 0;
            }

            for (size_t i = 0; i < 3; i++) {
                firework_t &rocket = data[i];

                if (rocket.state == 0) { // ready to start
                    rocket.state = 1;
                    rocket.sparkles[0].color = rgbToColor(255, 255, 255);
                    rocket.sparkles[0].bri = 1.0;
                    rocket.sparkles[0].pos = 0.0;

                    float height = random(700) / 10.0 + 50.0;
                    float vel = sqrt(10 * 10 + 2 * height * 9.81); // v0 = sqrt(v^2 - 2ha)

                    rocket.sparkles[0].vel = vel;
                } else if (rocket.state == 1) { // going up
                    size_t index = (size_t)(rocket.sparkles[0].pos * 60.0 / 50.0 / 180.0 * count);
                    if (index < count) {
                        uint8_t red = colorToRed(rocket.sparkles[0].color);
                        uint8_t green = colorToGreen(rocket.sparkles[0].color);
                        uint8_t blue = colorToBlue(rocket.sparkles[0].color);

                        float bri = rocket.sparkles[0].bri;

                        leds[index * 3] = (uint8_t)(red * bri);
                        leds[index * 3 + 1] = (uint8_t)(green * bri);
                        leds[index * 3 + 2] = (uint8_t)(blue * bri);
                    }

                    rocket.sparkles[0].pos += rocket.sparkles[0].vel * timeFac;
                    rocket.sparkles[0].vel += -9.81 * timeFac;

                    if (rocket.sparkles[0].vel < 10.0) {
                        // explode
                        float pos = rocket.sparkles[0].pos;

                        for (size_t i = 0; i < 10; i++) {
                            rocket.sparkles[i].pos = pos;
                            rocket.sparkles[i].vel = random(800) / 10.0 - 40.0; // from -40 to 40

                            float pos = random(1000) / 1000.0;
                            RgbColor color = palette->getColorAtPosition(pos);

                            rocket.sparkles[i].color = rgbToColor(color.R, color.G, color.B);
                            rocket.sparkles[i].bri = 1.0;
                        }

                        rocket.state = 2;
                    }
                } else if (rocket.state == 2) { // exploding sparkles fly
                    for (size_t i = 0; i < 10; i++) {
                        size_t index = (size_t)(rocket.sparkles[i].pos * 60.0 / 50.0 / 180.0 * count);
                        if (index < count) {
                            uint8_t red = colorToRed(rocket.sparkles[i].color);
                            uint8_t green = colorToGreen(rocket.sparkles[i].color);
                            uint8_t blue = colorToBlue(rocket.sparkles[i].color);

                            float bri = rocket.sparkles[i].bri;

                            leds[index * 3] = (uint8_t)(red * bri);
                            leds[index * 3 + 1] = (uint8_t)(green * bri);
                            leds[index * 3 + 2] = (uint8_t)(blue * bri);
                        }

                        rocket.sparkles[i].pos += rocket.sparkles[i].vel * timeFac;
                        rocket.sparkles[i].vel += -9.81 * timeFac;

                        rocket.sparkles[i].bri -= 1.0 / 2.5 * timeFac;
                        if (rocket.sparkles[i].bri < 0.0)
                            rocket.state = 3;
                    }
                } else if (rocket.state == 3) { // cooldown time
                    if (random(200) == 1)
                        rocket.state = 0;
                }
            }
        };
};

#endif

#ifndef FIREWORK_2D_H
#define FIREWORK_2D_H

#include "../../Effect.h"
#include "../../Utils.h"
#include "../../EffectManager.h"

struct sparkle_2d_t {
    uint32_t color;
    float bri;
    float posX;
    float posY;
    float velX;
    float velY;
};

struct firework_2d_t {
    int state;
    sparkle_2d_t sparkles[10];
};

class Firework2D : public SimulationEffect2D {
    private:
        firework_2d_t *data;

    public:
        Firework2D() : SimulationEffect2D(EFFECT_FIREWORK_2D) {
            data = NULL;
        };

        ~Firework2D() {
            if (data) {
                delete[] data;
                data = NULL;
            }
        }

        void init() {
            if (data) {
                delete[] data;
                data = NULL;
            }

            data = new firework_2d_t[3];
            for (size_t j = 0; j < 3; j++) {
                data[j].state = 0;
            }
        };

        void simulate(Palette *palette, unsigned long dTime) {
            float timeFac = dTime / 1000.0;

            float gravity = -9.81;

            for (size_t i = 0; i < 3; i++) {
                firework_2d_t &rocket = data[i];

                if (rocket.state == 0) { // ready to start
                    rocket.state = 1;
                    rocket.sparkles[0].color = rgbToColor(255, 255, 255);
                    rocket.sparkles[0].bri = 1.0;

                    float startX = random(150) - 75.0;
                    float startY = 0.0;

                    float destX = startX + random(40) - 20.0;
                    float destY = random(100) + 50;

                    float distX = destX - startX;
                    float distY = destY - startY;

                    float destVelY = -10.0;


                    float t = (-sqrtf(powf(destVelY, 2) - 2 * gravity * distY) + destVelY) / gravity;

                    float startVelY = destVelY - gravity * t;
                    float startVelX = distX / t;

                    rocket.sparkles[0].posX = startX;
                    rocket.sparkles[0].posY = startY;
                    rocket.sparkles[0].velX = startVelX;
                    rocket.sparkles[0].velY = startVelY;
                } else if (rocket.state == 1) { // going up
                    rocket.sparkles[0].posY += rocket.sparkles[0].velY * timeFac;
                    rocket.sparkles[0].posX += rocket.sparkles[0].velX * timeFac;                    

                    rocket.sparkles[0].velY += gravity * timeFac;

                    if (rocket.sparkles[0].velY < -10.0) {
                        // explode
                        float posX = rocket.sparkles[0].posX;
                        float posY = rocket.sparkles[0].posY;

                        for (size_t i = 0; i < 10; i++) {
                            rocket.sparkles[i].posX = posX;
                            rocket.sparkles[i].posY = posY;
                            rocket.sparkles[i].velX = random(40) - 20.0; // from -20.0 to 20.0
                            rocket.sparkles[i].velY = random(40) - 20.0;
                            
                            float pos = random(1000) / 1000.0;
                            RgbColor color = palette->getColorAtPosition(pos);

                            rocket.sparkles[i].color = rgbToColor(color.R, color.G, color.B);
                            rocket.sparkles[i].bri = 1.0;
                        }

                        rocket.state = 2;
                    }
                } else if (rocket.state == 2) { // exploding sparkles fly
                    for (size_t i = 0; i < 10; i++) {
                        rocket.sparkles[i].posX += rocket.sparkles[i].velX * timeFac;
                        rocket.sparkles[i].posY += rocket.sparkles[i].velY * timeFac;

                        rocket.sparkles[i].velY += gravity * timeFac;

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

        RgbColor colorAt(float xPos, float yPos, Palette *palette) {
            for (size_t i = 0; i < 3; i++) {
                firework_2d_t &rocket = data[i];

                if (rocket.state == 1 || rocket.state == 2) {
                    int size = rocket.state == 1 ? 1 : 10;

                    for (int j = 0; j < size; j++) {
                        float sparkelPosX = rocket.sparkles[j].posX / 75.0 * 40.0;
                        float sparklePosY = rocket.sparkles[j].posY / 150.0 * -30.0 + 15.0;


                        float dist = sqrt(pow(xPos - rocket.sparkles[j].posX, 2) + pow(yPos - sparklePosY, 2));

                        if (dist < 3.0) {
                            uint8_t red = colorToRed(rocket.sparkles[j].color);
                            uint8_t green = colorToGreen(rocket.sparkles[j].color);
                            uint8_t blue = colorToBlue(rocket.sparkles[j].color);

                            float bri = rocket.sparkles[j].bri;
                            bri *= 1.0 - dist / 3.0;

                            red = (uint8_t)(red * bri);
                            green = (uint8_t)(green * bri);
                            blue = (uint8_t)(blue * bri);

                            /*
                            Serial.print("Found color at ");
                            Serial.print(xPos);
                            Serial.print(" ");
                            Serial.print(yPos);
                            Serial.print(" (");
                            Serial.print(red);
                            Serial.print(", ");
                            Serial.print(green);
                            Serial.print(", ");
                            Serial.print(blue);
                            Serial.println(")");
                            */

                            return RgbColor(red, green, blue);
                        }
                    }
                }
            }

            return RgbColor(0, 0, 0);
        };
};

#endif

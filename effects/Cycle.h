#ifndef CYCLE_H
#define CYCLE_H

#include "../Effect.h"
#include "../PaletteManager.h"
#include "../Utils.h"

class Cycle : public Effect {
    public:
        Cycle() {
            Palette *palette = PaletteManager.createPalette("BlueAndRed");
            palette->addColorKey(0.0, rgbToColor(0, 0, 255));
            palette->addColorKey(1.0, rgbToColor(255, 0, 0));
        }

        void updateData() {

        }

        RgbColor update(double timeVal, double posVal) {
            double val = posVal - timeVal;
            if (val < 0.0) {
                val += 1.0;
            }

            Palette *palette = PaletteManager.getPalette("BlueAndRed");

            if (palette) {
                uint32_t color = palette->getColorAtPosition(val);
                uint8_t red = colorToRed(color);
                uint8_t green = colorToGreen(color);
                uint8_t blue = colorToBlue(color);

                return RgbColor(red, green, blue);
            }

            return RgbColor(0, 0, 0);
        }
};

#endif // CYCLE_H

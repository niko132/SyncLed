#ifndef GIF_2D_H
#define GIF_2D_H

#include "../Effect.h"
#include "../EffectManager.h"

#include "GifDec.h"

class Gif2D : public SimulationEffect2D {
    private:
        String _fileName;
        gd_GIF *_gif;
        long _milliTracker = 0;

    public:
        Gif2D() : SimulationEffect2D(EFFECT_GIF_2D) {
            Serial.println("GIF 2D Constructor");
            _fileName = "/gif/test2.gif";
        };

        ~Gif2D() {
            if (_gif) {
                gd_close_gif(_gif);
                _gif = NULL;
            }
        };


        void init() {
            _gif = gd_open_gif(_fileName.c_str());
            if (!_gif) {
                Serial.print("Failed opening GIF");
                return;
            }
        };

        void simulate(Palette *palette, unsigned long dTime) {
            if (!_gif)
                return;

            _milliTracker -= dTime;

            if (_milliTracker <= 0) {
                uint16_t delayMillis;

                if (gd_get_frame(_gif)) {
                    // gd_render_frame(_gif, _buffer);
                    delayMillis = _gif->gce.delay * 10;
                } else {
                    gd_rewind(_gif);
                    delayMillis = 0;
                }
                
                _milliTracker += delayMillis;
                if (_milliTracker < 0) {
                    _milliTracker = delayMillis; // reset
                }
            }
        };

        RgbColor colorAt(float xPos, float yPos, Palette *palette) {
            if (!_gif)
                return RgbColor(0, 0, 0);
            
            int xCoord, yCoord;
            mapCoords(xPos, yPos, _gif->width, _gif->height, xCoord, yCoord);

            uint8_t color[3];
            gd_get_color_at(_gif, xCoord, yCoord, color);

            uint8_t r = color[0];
            uint8_t g = color[1];
            uint8_t b = color[2];
            
            b *= 0.75;

            r = (uint8_t)(powf(r / 255.0, 2) * 255.0);
            g = (uint8_t)(powf(g / 255.0, 2) * 255.0);
            b = (uint8_t)(powf(b / 255.0, 2) * 255.0);

            RgbColor newColor(r, g, b);
            return newColor;
        };
};

#endif
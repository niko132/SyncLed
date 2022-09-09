#ifndef SPARKLE_FILL_H
#define SPARKLE_FILL_H

#include "../../Effect.h"
#include "../../EffectManager.h"

class SparkleFill : public SimulationEffect {
    private:
        unsigned long _period = 10000;
        unsigned long _elapsedMillis;
        bool *_filled = NULL;
        size_t _filledCount = 0;

    public:
        SparkleFill() : SimulationEffect(EFFECT_SPARKLE_FILL) {

        };

        void init(size_t oldLedCount, size_t newLedCount) {
            if (_filled) {
                delete[] _filled;
                _filled = NULL;
            }

            _filled = new bool[newLedCount];
            for (size_t i = 0; i < newLedCount; i++) {
                _filled[i] = false;
            }

            _elapsedMillis = 0;
        };

        void simulate(uint8_t *leds, size_t count, Palette *palette, unsigned long dTime) {
            _elapsedMillis += dTime;
            if (_elapsedMillis > _period) {
                _elapsedMillis -= (int)((double)_elapsedMillis / _period) * _period;
            }

            float frac = (double)_elapsedMillis / _period;
            float filledFrac = 2 * ((frac < 0.5f) ? frac : (1.0f - frac));

            size_t shouldFilled = count * (filledFrac);
            ssize_t diff = shouldFilled - _filledCount;

            while (diff != 0) { // TODO: change condition
                size_t idx = random(count);

                if (diff > 0) {
                    if (_filled[idx] == false) {
                        // fill
                        _filled[idx] = true;
                        diff--;
                    }
                } else {
                    if (_filled[idx] == true) {
                        // unfill
                        _filled[idx] = false;
                        diff++;
                    }
                }
            }

            _filledCount = shouldFilled;

            for (size_t i = 0; i < count; i++) {
                RgbColor color(0, 0, 0);
                if (_filled[i]) {
                    float pos = (float)i / count;
                    color = palette->getColorAtPosition(pos);
                }

                leds[i * 3] = color.R;
                leds[i * 3 + 1] = color.G;
                leds[i * 3 + 2] = color.B;
            }
        }
};

#endif
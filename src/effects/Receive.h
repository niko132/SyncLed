#ifndef RECEIVE_H
#define RECEIVE_H

#include "../Effect.h"
#include "../EffectManager.h"

#include "ESPAsyncUDP.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

class Receive : public SimulationEffect {
    private:
        uint16_t _port = 4648;
        bool _isListening = false;
        AsyncUDP _udp;

        uint8_t *_buf = NULL;
        size_t _ledCount = 0;

    public:
        Receive() : SimulationEffect(EFFECT_RECEIVE) {
            _udp.onPacket([this](AsyncUDPPacket packet) {
                if (packet.length() % 3 != 0) {
                    Serial.print("Received invalid UDP packet with size ");
                    Serial.println(packet.length());
                    return;
                }

                if (!_buf) {
                    return;
                }

                // copy from packet to _buf
                size_t len = MIN(packet.length(), _ledCount * 3);
                memcpy(_buf, packet.data(), len);
            });
        };

        ~Receive() {
            if (_buf) {
                delete[] _buf;
                _buf = NULL;
            }

            _udp.close();
        }

        void init(size_t oldLedCount, size_t newLedCount) {
            if (_buf) {
                delete[] _buf;
                _buf = NULL;
            }

            _buf = new uint8_t[3 * newLedCount]; // rgb
            _ledCount = newLedCount;
        };

        void simulate(uint8_t *leds, size_t count, Palette *palette, unsigned long dTime) {
            if (!_buf) {
                return;
            }
            
            if (!_isListening) {
                _isListening = _udp.listen(_port);
            } else {
                size_t len = (MIN(count, _ledCount)) * 3;
                memcpy(leds, _buf, len);
            }
        };

        void fromJson(JsonObject &root) {
            uint16_t port = root["p"] | 0;
            if (port && port != _port) {
                _port = port;
                _isListening = false;
            }
        };

        void toJson(JsonObject &root) {
            root["p"] = _port;
        };

};

#endif
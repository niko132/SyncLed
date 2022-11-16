#ifndef RECEIVE_2D_H
#define RECEIVE_2D_H

#include "../Effect.h"
#include "../EffectManager.h"

#include "ESPAsyncUDP.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

class Receive2D : public SimulationEffect2D {
    private:
        uint16_t _port = 4648;
        bool _isListening = false;
        AsyncUDP _udp;

        uint8_t *_buf = NULL;
        size_t _width = 25;
        size_t _height = 10;

        float _minBri = 0.01f;

    public:
        Receive2D() : SimulationEffect2D(EFFECT_RECEIVE_2D) {
            _udp.onPacket([this](AsyncUDPPacket packet) {
                if (packet.length() % 3 != 0 || packet.length() != 3 * _width * _height) {
                    Serial.print("Received invalid UDP packet with size ");
                    Serial.println(packet.length());
                    return;
                }

                if (!_buf) {
                    return;
                }

                // copy from packet to _buf
                size_t len = packet.length();
                memcpy(_buf, packet.data(), len);
            });
        };

        ~Receive2D() {
            if (_buf) {
                delete[] _buf;
                _buf = NULL;
            }

            _udp.close();
        }

        void init() {
            if (!_buf) {
                _buf = new uint8_t[3 * _width * _height]; // rgb
            }
        };

        void simulate(Palette *palette, unsigned long dTime) {
            if (!_isListening) {
                _isListening = _udp.listen(_port);
            }
        };

        RgbColor colorAt(float xPos, float yPos, Palette *palette) {
            if (!_buf) {
                return RgbColor();
            }

            int x, y;
            mapCoords(xPos, yPos, _width, _height, x, y);

            size_t index = y * _width + x;
            RgbColor rgb(_buf[index * 3], _buf[index * 3 + 1], _buf[index * 3 + 2]);
            HsbColor hsb(rgb);
            hsb.B = hsb.B * (1.0f - _minBri) + _minBri; // TODO: implement controlable min brightnes adjustment
            return RgbColor(hsb);
        }

        void fromJson(JsonObject &root) {
            uint16_t port = root["p"] | 0;
            if (port && port != _port) {
                _port = port;
                _isListening = false;
            }

            size_t width = root["w"] | _width;
            size_t height = root["h"] | _height;
            if (width != _width || height != _height) {
                _width = width;
                _height = height;

                if (_buf) {
                    delete[] _buf;
                    _buf = NULL;
                }

                _buf = new uint8_t[3 * _width * _height];
            }
        };

        void toJson(JsonObject &root) {
            root["p"] = _port;
            root["w"] = _width;
            root["h"] = _height;
        };

};

#endif
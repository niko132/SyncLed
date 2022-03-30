#ifndef UTILS_H
#define UTILS_H

#include <arduino.h>
#include <NeoPixelBus.h>

#include <ArduinoJson.h>
#include <LittleFS.h>

static uint8_t colorToRed(uint32_t color) {
    return (color>>16) & 255;
}

static uint8_t colorToGreen(uint32_t color) {
    return (color>>8) & 255;
}

static uint8_t colorToBlue(uint32_t color) {
    return color & 255;
}

static uint32_t rgbToColor(uint8_t red, uint8_t green, uint8_t blue) {
    uint32_t color = 0;
    color |= red<<16 | green<<8 | blue;
    return color;
}

static RgbColor colorToRgbColor(uint32_t color) {
    uint8_t r = colorToRed(color);
    uint8_t g = colorToGreen(color);
    uint8_t b = colorToBlue(color);

    return RgbColor(r, g, b);
}

static bool readJson(String path, DynamicJsonDocument &doc) {
    if (!LittleFS.exists(path)) {
        doc.to<JsonObject>();
        return false;
    }

    File f = LittleFS.open(path, "r");
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.print("Deserialization failed: ");
        Serial.println(err.c_str());
        doc.to<JsonObject>();
        return false;
    }

    Serial.println("Deserialization complete");
    return true;
}

class Reader {
    private:
        uint8_t *_data = NULL;
        size_t _length = 0;

        size_t _currentPos = 0;

    public:
        Reader(uint8_t *data, size_t length) {
            _data = data;
            _length = length;

            _currentPos = 0;
        }

        bool read(uint8_t *buf, size_t length) {
            if (_currentPos + length <= _length) {
                memcpy(buf, &_data[_currentPos], length);
                _currentPos += length;

                return true;
            }

            return false;
        }

        template<typename T>
        bool read(T *buf) {
            return read((uint8_t*) buf, sizeof(T));
        }

        // template<>
        bool read(String &buf) {
            buf = "";
            char c = 1; // use non 0 value

            while (read<char>(&c) && c != 0) {
                buf += c;
            }

            return c == 0;
        }

        uint8_t* getRemainingData(size_t *length) {
            *length = _length - _currentPos;
            return &_data[_currentPos];
        }
};

class Writer {
    private:
        uint8_t *_buf = NULL;
        size_t _length = 0;

        size_t _currentPos = 0;

        bool reallocate(size_t length) {
            if (_length < length) {
                uint8_t *tmpBuf = new uint8_t[length];

                if (tmpBuf == NULL) {
                    return false;
                }

                if (_buf) {
                    memcpy(tmpBuf, _buf, _length);
                    delete[] _buf;
                }

                _buf = tmpBuf;
                _length = length;
            }

            return true;
        }

        bool reallocateIfNeeded(size_t dataLength) {
            if (_currentPos + dataLength > _length) {
                size_t newLength = _length;
                do {
                    newLength *= 2;
                } while(_currentPos + dataLength > newLength);

                return reallocate(newLength);
            }

            return true;
        }

    public:
        Writer(size_t length = 16) {
            _currentPos = 0;
            reallocate(length);
        }

        ~Writer() {
            if (_buf) {
                delete[] _buf;
                _buf = NULL;
            }
        }

        bool write(uint8_t *data, size_t length) {
            if (reallocateIfNeeded(length)) {
                memcpy(&_buf[_currentPos], data, length);
                _currentPos += length;

                return true;
            }

            return false;
        }

        template<typename T>
        bool write(T data) {
            return write((uint8_t*) &data, sizeof(T));
        }

        // template<>
        bool write(String buf) {
            return write((uint8_t*)buf.c_str(), buf.length() + 1);
        }

        uint8_t* getData(size_t *length) {
            *length = _currentPos;
            return _buf;
        }

        uint8_t* getData() {
            return _buf;
        }

        size_t getLength() {
            return _currentPos;
        }
};

#endif // UTILS_H

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
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


static float wrapValue(float value, float range) {
    return value - range * floorf(value / range);
}

static float wrapValue(float value) {
    return wrapValue(value, 1.0);
}


static size_t size = 232;
static float xVals[] = {48.75, 48.75, 48.75, 48.75, 48.75, 48.75, 48.75, 48.75, 45.83, 44.17, 42.50, 40.83, 39.17, 37.50, 35.83, 34.17, 31.25, 31.25, 31.25, 31.25, 31.25, 31.25, 31.25, 31.25, 34.17, 35.83, 37.50, 39.17, 40.83, 42.50, 44.17, 45.83, 48.75, 48.75, 48.75, 48.75, 48.75, 48.75, 48.75, 48.75, 45.83, 44.17, 42.50, 40.83, 39.17, 37.50, 35.83, 34.17, 31.25, 31.25, 31.25, 31.25, 31.25, 31.25, 31.25, 31.25, 26.25, 26.25, 26.25, 26.25, 26.25, 26.25, 26.25, 26.25, 23.33, 21.67, 20.00, 18.33, 16.67, 15.00, 13.33, 11.67, 8.75, 8.75, 8.75, 8.75, 8.75, 8.75, 8.75, 8.75, 11.67, 13.33, 15.00, 16.67, 18.33, 20.00, 21.67, 23.33, 26.25, 26.25, 26.25, 26.25, 26.25, 26.25, 26.25, 26.25, 23.33, 21.67, 20.00, 18.33, 16.67, 15.00, 13.33, 11.67, 8.75, 8.75, 8.75, 8.75, 8.75, 8.75, 8.75, 8.75, 0.83, -0.83, -0.83, 0.83, 0.83, -0.83, -0.83, 0.83, -8.75, -8.75, -8.75, -8.75, -8.75, -8.75, -8.75, -8.75, -11.67, -13.33, -15.00, -16.67, -18.33, -20.00, -21.67, -23.33, -26.25, -26.25, -26.25, -26.25, -26.25, -26.25, -26.25, -26.25, -23.33, -21.67, -20.00, -18.33, -16.67, -15.00, -13.33, -11.67, -8.75, -8.75, -8.75, -8.75, -8.75, -8.75, -8.75, -8.75, -11.67, -13.33, -15.00, -16.67, -18.33, -20.00, -21.67, -23.33, -26.25, -26.25, -26.25, -26.25, -26.25, -26.25, -26.25, -26.25, -31.25, -31.25, -31.25, -31.25, -31.25, -31.25, -31.25, -31.25, -34.17, -35.83, -37.50, -39.17, -40.83, -42.50, -44.17, -45.83, -48.75, -48.75, -48.75, -48.75, -48.75, -48.75, -48.75, -48.75, -45.83, -44.17, -42.50, -40.83, -39.17, -37.50, -35.83, -34.17, -31.25, -31.25, -31.25, -31.25, -31.25, -31.25, -31.25, -31.25, -34.17, -35.83, -37.50, -39.17, -40.83, -42.50, -44.17, -45.83, -48.75, -48.75, -48.75, -48.75, -48.75, -48.75, -48.75, -48.75};
static float yVals[] = {-2.92, -4.58, -6.25, -7.92, -9.58, -11.25, -12.92, -14.58, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -14.58, -12.92, -11.25, -9.58, -7.92, -6.25, -4.58, -2.92, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 2.92, 4.58, 6.25, 7.92, 9.58, 11.25, 12.92, 14.58, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 14.58, 12.92, 11.25, 9.58, 7.92, 6.25, 4.58, 2.92, -2.92, -4.58, -6.25, -7.92, -9.58, -11.25, -12.92, -14.58, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -14.58, -12.92, -11.25, -9.58, -7.92, -6.25, -4.58, -2.92, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 2.92, 4.58, 6.25, 7.92, 9.58, 11.25, 12.92, 14.58, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 14.58, 12.92, 11.25, 9.58, 7.92, 6.25, 4.58, 2.92, -4.55, -4.55, -2.9, -2.9, 2.9, 2.9, 4.55, 4.55, -2.92, -4.58, -6.25, -7.92, -9.58, -11.25, -12.92, -14.58, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -14.58, -12.92, -11.25, -9.58, -7.92, -6.25, -4.58, -2.92, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 2.92, 4.58, 6.25, 7.92, 9.58, 11.25, 12.92, 14.58, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 14.58, 12.92, 11.25, 9.58, 7.92, 6.25, 4.58, 2.92, -2.92, -4.58, -6.25, -7.92, -9.58, -11.25, -12.92, -14.58, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -17.50, -14.58, -12.92, -11.25, -9.58, -7.92, -6.25, -4.58, -2.92, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 2.92, 4.58, 6.25, 7.92, 9.58, 11.25, 12.92, 14.58, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 17.50, 14.58, 12.92, 11.25, 9.58, 7.92, 6.25, 4.58, 2.92};

// precalculate the bounds to safe time later
static bool isInitialized = false;
static float minX, minY, maxX, maxY;

static bool getLedPosition2D(size_t index, float &xPos, float &yPos) {
    if (index >= size || index >= size) {
        return false;
    }

    xPos = xVals[index];
    yPos = yVals[index];

    return true;
}

static void initialize() {
    minX = maxX = xVals[0];
    minY = maxY = yVals[0];

    for (float val : xVals) {
        if (val < minX)
            minX = val;
        if (val > maxX)
            maxX = val;
    }

    for (float val : yVals) {
        if (val < minY)
            minY = val;
        if (val > maxY)
            maxY = val;
    }

    isInitialized = true;
}

static void absToRelCoords(float absX, float absY, float &relX, float &relY) {
    if (!isInitialized) {
        initialize();
    }

    relX = (absX - minX) / (maxX - minX);
    relY = (absY - minY) / (maxY - minY);
}

static void mapCoords(float absX, float absY, int xDim, int yDim, int &outX, int &outY) {
    float relX, relY;
    absToRelCoords(absX, absY, relX, relY);

    outX = (int)(relX * (xDim - 1) + 0.5f);
    outY = (int)(relY * (yDim - 1) + 0.5f);
}


#endif // UTILS_H

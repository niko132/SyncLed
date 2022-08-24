#ifndef VIRTUAL_DEVICE_MANAGER_H
#define VIRTUAL_DEVICE_MANAGER_H

#include <NeoPixelBus.h>
#include <map>
#include <ArduinoJson.h>

#include "VirtualDevice.h"

#define LED_COUNT 232
#define UPDATE_MILLIS 17

#define LIVE_UPDATE_MILLIS 75

typedef std::map<unsigned long, VirtualDevice*> vd_map;
typedef vd_map::iterator vd_map_itr;

enum State {
    OFF,
    FADE_IN,
    ON,
    FADE_OUT
};

const uint8_t BRI_LOOKUP[] = {0,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,24,24,25,25,26,27,27,28,29,29,30,31,31,32,33,33,34,35,36,36,37,38,39,39,40,41,42,42,43,44,45,46,47,47,48,49,50,51,52,53,54,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,78,79,80,81,82,83,84,85,87,88,89,90,91,92,94,95,96,97,99,100,101,102,104,105,106,107,109,110,111,113,114,115,117,118,119,121,122,123,125,126,128,129,130,132,133,135,136,138,139,141,142,143,145,146,148,150,151,153,154,156,157,159,160,162,164,165,167,168,170,172,173,175,177,178,180,182,183,185,187,188,190,192,194,195,197,199,201,202,204,206,208,209,211,213,215,217,219,220,222,224,226,228,230,232,234,235,237,239,241,243,245,247,249,251,253,255};

class ESPVirtualDeviceManager {
    private:
        NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> _strip;
        vd_map _virtualDevices;
        uint8_t _colors[LED_COUNT * 3];
        unsigned long _lastUpdateMillis = 0;
        unsigned long _lastLiveUpdateMillis = 0;

        State _state = OFF;
        float _currentFadeBri = 0.0;

        bool _useBrightnessCorrection = false;

        bool _useBrightnessControl = false;

        size_t _numBrightnessControlEntries = 0;
        float *_brightnessControlEntries = nullptr;

    public:
        ESPVirtualDeviceManager();
        void begin();
        void update();

        vd_map_itr vdBegin();
        vd_map_itr vdEnd();

        void fadeIn();
        void fadeOut();

        void setOn(bool on);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

extern ESPVirtualDeviceManager VirtualDeviceManager;

#endif

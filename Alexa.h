#ifndef ALEXA_H
#define ALEXA_H

#define ESPALEXA_ASYNC //it is important to define this before #include <Espalexa.h>!
#include <Espalexa.h>
#include <ArduinoJson.h>

class ESPAlexa {
    private:
        Espalexa _alexa;
        EspalexaDevice *_alexaDevice;
        String _name;

    public:
        void begin();
        void update();

        void deviceCallback(EspalexaDevice* d);
        void registerDevice(EspalexaDevice *d);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

extern ESPAlexa Alexa;

#endif

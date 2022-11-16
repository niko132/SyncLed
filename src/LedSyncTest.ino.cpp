#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "secrets.h"

#include "ESPAsyncUDP.h"

#include "NetworkManager.h"
#include "ConnectivityManager.h"
#include "TimeManager.h"

#include <NeoPixelBus.h>
#include "VirtualDeviceManager.h"
#include "PaletteManager.h"
#include "PresetManager.h"
#include "StorageManager.h"
#include "WebManager.h"
#include "OverlayManager.h"

#include "WifiManager.h"

#include "Alexa.h"

#include <FS.h>
#include <LittleFS.h>

const char *ssid = WIFI_SSID;
const char *pass = WIFI_PASS;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    delay(1000);
    Serial.println("Booting");


    // position calculation util
    /*
    float startX = 48.75;
    float startY = 0.0;

    float offset = (17.5 - 7 * 100.0 / 60.0) / 2.0;

    float xFact[] = {0.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 0.0};
    float yFact[] = {-1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, -1.0};

    float xFact1[] = {0.0, -1.0, 0.0, 1.0, 0.0, -1.0, 0.0};
    float yFact1[] = {-1.0, 0.0, 1.0, 0.0, 1.0, 0.0, -1.0};

    for (int i = 0; i < 7; i++) {
        float currXFac = xFact1[i];
        float currYFac = yFact1[i];

        startX += currXFac * offset;
        startY += currYFac * offset;

        for (int j = 0; j < 8; j++) {
            float x = startX;
            float y = startY;

            Serial.print(y);
            Serial.print(", ");

            if (j == 7) {
                break;
            }

            startX += 100.0 / 60.0 * currXFac;
            startY += 100.0 / 60.0 * currYFac;
        }

        startX += currXFac * offset;
        startY += currYFac * offset;
    }

    startX -= 5.0;

    for (int i = 0; i < 7; i++) {
        float currXFac = xFact1[i];
        float currYFac = yFact1[i];

        startX += currXFac * offset;
        startY += currYFac * offset;

        for (int j = 0; j < 8; j++) {
            float x = startX;
            float y = startY;

            Serial.print(y);
            Serial.print(", ");

            if (j == 7) {
                break;
            }

            startX += 100.0 / 60.0 * currXFac;
            startY += 100.0 / 60.0 * currYFac;
        }

        startX += currXFac * offset;
        startY += currYFac * offset;
    }

    Serial.print("-2.0, -5.0, -5.0, -2.0, 2.0, 5.0, 5.0, 2.0, ");

    startX = -startX;

    for (int i = 0; i < 7; i++) {
        float currXFac = xFact1[i];
        float currYFac = yFact1[i];

        startX += currXFac * offset;
        startY += currYFac * offset;

        for (int j = 0; j < 8; j++) {
            float x = startX;
            float y = startY;

            Serial.print(y);
            Serial.print(", ");

            if (j == 7) {
                break;
            }

            startX += 100.0 / 60.0 * currXFac;
            startY += 100.0 / 60.0 * currYFac;
        }

        startX += currXFac * offset;
        startY += currYFac * offset;
    }

    startX -= 5.0;

    for (int i = 0; i < 7; i++) {
        float currXFac = xFact1[i];
        float currYFac = yFact1[i];

        startX += currXFac * offset;
        startY += currYFac * offset;

        for (int j = 0; j < 8; j++) {
            float x = startX;
            float y = startY;

            Serial.print(y);
            Serial.print(", ");

            if (j == 7) {
                break;
            }

            startX += 100.0 / 60.0 * currXFac;
            startY += 100.0 / 60.0 * currYFac;
        }

        startX += currXFac * offset;
        startY += currYFac * offset;
    }


    Serial.println();
    Serial.println("Finished");
    */



    /*
    WiFi.disconnect();
    // WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, pass);
    Serial.print("Connecting to: ");
    Serial.println(ssid);

    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("...");
    }

    Serial.println("Connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    */

    WifiManager.autoConnect();



    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin(true);

    delay(1000);

    if(!LittleFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
    }


    NetworkManager.begin();
    ConnectivityManager.begin();
    TimeManager.begin();
    // VirtualDeviceManager.begin();
    PresetManager.begin();
    WebManager.begin();
    OverlayManager.begin();

    Alexa.begin();

    StorageManager.loadConfig();
    VirtualDeviceManager.begin();


    Serial.println("Setup");
}

int state = 1;

void loop() {
    // put your main code here, to run repeatedly:
    ArduinoOTA.handle();

    ConnectivityManager.update();
    TimeManager.update();
    VirtualDeviceManager.update();
    PresetManager.update();
    StorageManager.update();
    Alexa.update();

    if (state == 1) {
        if (TimeManager.canBeVisible()) {
            VirtualDeviceManager.fadeIn();
            state = 2;
        }
    } else if (state == 2) {
        // fading in
    }
}

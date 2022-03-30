#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include "ESPAsyncWebServer.h"

#define CONNECT_TIMEOUT_MILLIS 15 * 1000
#define DNS_PORT 53

class WifiConfig {
    public:
        String ssid;
        String password;
        IPAddress ip;
        IPAddress gateway;
        IPAddress subnet;

        WifiConfig();
        bool hasCredentials();
        bool hasIp();

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

class ESPWifiManager {
    private:
        DNSServer _dnsServer;

        IPAddress _apIP;

        WifiConfig loadConfig();
        void storeConfig(WifiConfig config);
        bool connect(WifiConfig config);
        void startAP();
        void setupDNS();

        void onScanComplete(int numNetworks);

    public:
        ESPWifiManager();
        void autoConnect();

        void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};

extern ESPWifiManager WifiManager;

#endif

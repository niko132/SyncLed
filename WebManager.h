#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"

class ESPWebManager {
    private:
        AsyncWebServer _server;
        AsyncWebSocket _ws;
        AsyncWebSocket _wifiConfigWs;
        
        AsyncWebSocket _liveDataWs;

        File _currentFile;

    public:
        ESPWebManager();

        void begin();
        AsyncWebServer* getServer();
        void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

        void sendToAllClients(String text);
        void sendToAllWifiConfigClients(String text);

        void sendLiveData(uint8_t *buf, unsigned long length);
};

extern ESPWebManager WebManager;

#endif

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "Utils.h"

#define TIME_SYNC_INTERVAL 10000
#define SINGLE_HOST_TIME 2000

#define SUBTYPE_SYNC_REQUEST 1
#define SUBTYPE_SYNC_RESPONSE 2

class ESPTimeManager {
	private:
		unsigned long _offsetMillis = 0;
		unsigned long _lastSyncMillis = 0;

		bool _synced = false;
		unsigned long _beginMillis = 0;

		WiFiUDP _ntpUdp;
        NTPClient _timeClient;

	public:
		ESPTimeManager();
		void begin();
		void update();
		bool sendTimeSyncRequest();
		unsigned long syncedMillis();

		bool handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis);

		bool canBeVisible();

		int getHours();
		int getMinutes();
		int getSeconds();
};

extern ESPTimeManager TimeManager;

#endif

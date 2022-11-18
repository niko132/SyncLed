#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <ESP8266WiFi.h>
#include "Utils.h"
#include <time.h>
#include <TZ.h>

#define MY_TZ TZ_Europe_Berlin

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

	public:
		ESPTimeManager();
		void begin();
		void update();
		bool sendTimeSyncRequest();
		unsigned long syncedMillis();

		bool handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis);

		bool canBeVisible();

		tm getCurrentTime();

		int getHours();
		int getMinutes();
		int getSeconds();
};

extern ESPTimeManager TimeManager;

#endif

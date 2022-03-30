#ifndef CONNECTIVITY_MANAGER_H
#define CONNECTIVITY_MANAGER_H

#include <ESP8266WiFi.h>
#include <map>
#include <set>
#include <ArduinoJson.h>

#include "Utils.h"

#define HEARTBEAT_INTERVAL 2000

#define SUBTYPE_HEARTBEAT_REQUEST 1
#define SUBTYPE_HEARTBEAT_RESPONSE 2

namespace std {
    template<>
    struct less<IPAddress> {
        public:
            bool operator()(const IPAddress &lhs, const IPAddress &rhs) const {
                return std::less<String>()(lhs.toString(), rhs.toString());
            }
    };
}

typedef std::map<IPAddress, unsigned long> act_dev_map;
typedef act_dev_map::iterator act_dev_map_itr;

class ESPConnectivityManager {
	private:
		unsigned long _lastHeartbeatMillis = 0;
		act_dev_map _activeDevices;

        void sendHeartbeatRequest();
		void sendHeartbeat();
		void removeInactive();

	public:
		void begin();
		void update();

		size_t getActiveDeviceCount();
		std::set<IPAddress> getActiveDevices();

		bool handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis);

        void sendDevices();
        void toJson(JsonObject &root);
};

extern ESPConnectivityManager ConnectivityManager;

#endif

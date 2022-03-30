#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "ESPAsyncUDP.h"

#define PORT_UDP 4867

#define TYPE_CONNECTIVITY 1
#define TYPE_TIME 2
#define TYPE_PRESET 3

class ESPNetworkManager {
	private:
		AsyncUDP _udp;

	public:
		void begin();
		void sendTo(uint8_t *data, size_t len, IPAddress ip);
		void broadcast(uint8_t *data, size_t len);
};

extern ESPNetworkManager NetworkManager;

#endif

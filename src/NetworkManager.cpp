#include "NetworkManager.h"

#include "ConnectivityManager.h"
#include "TimeManager.h"
#include "PresetManager.h"

void ESPNetworkManager::begin() {
	_udp.onPacket([](AsyncUDPPacket packet) {
		unsigned long receiveMillis = TimeManager.syncedMillis();
		Reader reader(packet.data(), packet.length());

		uint8_t type;
		reader.read(&type);

		if (type == TYPE_CONNECTIVITY) {
			ConnectivityManager.handlePacket(reader, packet.remoteIP(), receiveMillis);
		} else if (type == TYPE_TIME) {
			TimeManager.handlePacket(reader, packet.remoteIP(), receiveMillis);
		} else if (type == TYPE_PRESET) {
			PresetManager.handlePacket(reader, packet.remoteIP(), receiveMillis);
		}
	});
	_udp.listen(PORT_UDP);
}

void ESPNetworkManager::sendTo(uint8_t *data, size_t len, IPAddress ip) {
	_udp.writeTo(data, len, ip, PORT_UDP);
}

void ESPNetworkManager::broadcast(uint8_t *data, size_t len) {
	_udp.broadcastTo(data, len, PORT_UDP);
}

ESPNetworkManager NetworkManager;

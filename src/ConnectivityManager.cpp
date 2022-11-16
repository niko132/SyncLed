#include "ConnectivityManager.h"

#include "NetworkManager.h"
#include "TimeManager.h"
#include "WebManager.h"

void ESPConnectivityManager::begin() {
	_lastHeartbeatMillis = millis() - HEARTBEAT_INTERVAL;
	sendHeartbeatRequest();
}

void ESPConnectivityManager::update() {
	unsigned long now = millis();

	if (TimeManager.canBeVisible() && now - _lastHeartbeatMillis > HEARTBEAT_INTERVAL) {
		sendHeartbeat();
		_lastHeartbeatMillis = now;
	}

	removeInactive();
}

size_t ESPConnectivityManager::getActiveDeviceCount() {
	return _activeDevices.size();
}

std::set<IPAddress> ESPConnectivityManager::getActiveDevices() {
	std::set<IPAddress> devices;

	for (act_dev_map_itr it = _activeDevices.begin(); it != _activeDevices.end(); it++) {
		devices.insert(it->first);
	}

	return devices;
}

bool ESPConnectivityManager::handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis) {
	uint8_t subType;
	reader.read(&subType);

	if (subType == SUBTYPE_HEARTBEAT_REQUEST) {
		sendHeartbeat();
	} else if (subType == SUBTYPE_HEARTBEAT_RESPONSE) {
		bool newDevice = false;

		if (!_activeDevices.count(ip)) {
			Serial.println("New Device");
			newDevice = true;
		}

		_activeDevices[ip] = millis();

		if (newDevice) {
			sendDevices();
		}
	} else {
		return false;
	}

	return true;
}

void ESPConnectivityManager::sendHeartbeatRequest() {
	Writer writer;

	uint8_t type = TYPE_CONNECTIVITY;
	writer.write(type);
	uint8_t subType = SUBTYPE_HEARTBEAT_REQUEST;
	writer.write(subType);

	NetworkManager.broadcast(writer.getData(), writer.getLength());
}

void ESPConnectivityManager::sendHeartbeat() {
	Writer writer;

	uint8_t type = TYPE_CONNECTIVITY;
	writer.write(type);
	uint8_t subType = SUBTYPE_HEARTBEAT_RESPONSE;
	writer.write(subType);

	NetworkManager.broadcast(writer.getData(), writer.getLength());
}

void ESPConnectivityManager::removeInactive() {
	unsigned long now = millis();
	act_dev_map_itr it = _activeDevices.begin();
	bool deleted = false;

	while(it != _activeDevices.end()) {
		std::map<IPAddress, unsigned long>::iterator current = it++;

		if (now - current->second > 2.5 * HEARTBEAT_INTERVAL) {
			_activeDevices.erase(current);
			Serial.print(current->first);
			Serial.print(" off (");
      		Serial.print(now - current->second);
      		Serial.println("ms)");

			deleted = true;
		}
	}

	if (deleted) {
		sendDevices();
	}
}

void ESPConnectivityManager::sendDevices() {
	DynamicJsonDocument doc(5120);
	JsonObject jsonObj = doc.to<JsonObject>();

	ConnectivityManager.toJson(jsonObj);

	String json;
	serializeJson(doc, json);
	WebManager.sendToAllClients(json);
}

void ESPConnectivityManager::toJson(JsonObject &root) {
	JsonArray actDevArray = root.createNestedArray("actDev");

	for (act_dev_map_itr it = _activeDevices.begin(); it != _activeDevices.end(); it++) {
		IPAddress ip = it->first;
		actDevArray.add(ip.toString());
	}
}

ESPConnectivityManager ConnectivityManager;

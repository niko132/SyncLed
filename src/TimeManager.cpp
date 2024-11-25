#include "TimeManager.h"

#include "NetworkManager.h"
#include "ConnectivityManager.h"
#include "Utils.h"

ESPTimeManager::ESPTimeManager() {
	configTime(MY_TZ, "pool.ntp.org");
}

void ESPTimeManager::begin() {
	_lastSyncMillis = millis() - TIME_SYNC_INTERVAL;
	_beginMillis = millis();
}

void ESPTimeManager::update() {
	unsigned long now = millis();

	if (now - _lastSyncMillis > TIME_SYNC_INTERVAL) {
		if (sendTimeSyncRequest())
			_lastSyncMillis = now;
	}
}

bool ESPTimeManager::sendTimeSyncRequest() {
	size_t deviceCount = ConnectivityManager.getActiveDeviceCount();
	if (deviceCount <= 0) {
		return false;
	}

	std::set<IPAddress> ips = ConnectivityManager.getActiveDevices();
	std::set<IPAddress>::iterator it = ips.begin();

	int r = random(deviceCount);
	while(r--) {
		it++;
	}

	IPAddress ip = *it;
	Writer writer;

	uint8_t type = TYPE_TIME;
	writer.write(type);
	uint8_t subType = SUBTYPE_SYNC_REQUEST; // sync request
	writer.write(subType);
	unsigned long millis = syncedMillis();
	writer.write(millis);

	NetworkManager.sendTo(writer.getData(), writer.getLength(), ip);

	return true;
}

unsigned long ESPTimeManager::syncedMillis() {
	return millis() + _offsetMillis;
}

bool ESPTimeManager::handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis) {
	uint8_t subType;
	reader.read(&subType);

	if (subType == SUBTYPE_SYNC_REQUEST) { // time request
		unsigned long clientStart;
		reader.read(&clientStart);

		Writer writer;
		uint8_t type = TYPE_TIME;
		writer.write(type);
		uint8_t subType = SUBTYPE_SYNC_RESPONSE; // time response
		writer.write(subType);

		writer.write(clientStart);
		writer.write(receiveMillis);
		unsigned long serverEnd = syncedMillis();
		writer.write(serverEnd);

		NetworkManager.sendTo(writer.getData(), writer.getLength(), ip);
	} else if (subType == SUBTYPE_SYNC_RESPONSE) { // time response
		unsigned long clientStart;
		unsigned long serverStart;
		unsigned long serverEnd;
		unsigned long clientEnd = receiveMillis;

		reader.read(&clientStart);
		reader.read(&serverStart);
		reader.read(&serverEnd);

		long offset = (long)((long)(serverStart - clientStart + serverEnd - clientEnd) / 2.0 + 0.5);
		unsigned long rtt = clientEnd - clientStart - serverEnd + serverStart;

		Serial.print("Time offset: ");
		Serial.println(offset);
		Serial.print("RTT: ");
		Serial.println(rtt);

		_offsetMillis += offset;
		_synced = true;
	} else {
		return false;
	}

	return true;
}

bool ESPTimeManager::canBeVisible() {
	return _synced || millis() - _beginMillis > SINGLE_HOST_TIME;
}

tm ESPTimeManager::getCurrentTime() {
	time_t now;
	tm tm;
	time(&now);
	localtime_r(&now, &tm);

	return tm;
}

int ESPTimeManager::getHours() {
	return getCurrentTime().tm_hour;
}

int ESPTimeManager::getMinutes() {
	return getCurrentTime().tm_min;
}

int ESPTimeManager::getSeconds() {
	return getCurrentTime().tm_sec;
}


ESPTimeManager TimeManager;

#include "PresetManager.h"

#include "StorageManager.h"
#include "WebManager.h"

using namespace std::placeholders;

Preset::Preset(unsigned long id) {
    _id = id;

    uint32_t localIP = WiFi.localIP();
    _ips.insert(localIP);

    _enableAlexa = false;
};

Preset::Preset(unsigned long id, String name, ip_set ips) {
    _id = id;
    _name = name;
    _ips = ips;
};

unsigned long Preset::getId() {
    return _id;
};

String Preset::getName() {
    return _name;
};

void Preset::load() {
    Writer writer;
    uint8_t type = TYPE_PRESET;
    writer.write(type);
    uint8_t subType = SUBTYPE_SET_PRESET;
    writer.write(subType);
    writer.write(_id);

    uint32_t localIP = WiFi.localIP();

    for (ip_set_itr it = _ips.begin(); it != _ips.end(); it++) {
        uint32_t ip = *it;
        if (ip != localIP) {
            IPAddress ipAddr(ip);
            NetworkManager.sendTo(writer.getData(), writer.getLength(), ipAddr);
        }
    }
};

void Preset::alexaDeviceCallback(EspalexaDevice* d) {
    if (d == NULL)
        return;
    
    if (d->getValue()) {
        // activate the preset
        Serial.println("Alexa - activate preset: " + _name);
        PresetManager.load(_id);
    }
};

void Preset::fromJson(JsonObject &root) {
    _id = root["id"] | _id;
    _name = root["n"] | _name;

    JsonArray ipsArray = root["ips"];
    if (ipsArray) {
        _ips.clear();
        uint32_t localIP = WiFi.localIP();
        _ips.insert(localIP);
        for (JsonVariant ipVar : ipsArray) {
            String ipStr = ipVar.as<String>();
            IPAddress ipAddr;
            ipAddr.fromString(ipStr);
            uint32_t ip = ipAddr;
            _ips.insert(ip);
        }
    }

    _enableAlexa = root["eA"] | _enableAlexa;
    JsonVariant enableAlexaVar = root["eA"];
    if (!enableAlexaVar.isNull()) {
        // enable
        if (_enableAlexa) {
            if (!_alexaDevice) {
                Serial.println("Creating new AlexaDevice: " + _name);
                _alexaDevice = new EspalexaDevice(_name, std::bind(&Preset::alexaDeviceCallback, this, _1), EspalexaDeviceType::onoff);
                Alexa.registerDevice(_alexaDevice);
            } else {
                Serial.println("Changing name of AlexaDevice: " + _name);
                _alexaDevice->setName(_name);
            }
        } else {
            if (_alexaDevice) {
                Serial.println("Deleting AlexaDevice");
                // TODO: can cause NPE - find another way of removing
                delete _alexaDevice;
                _alexaDevice = NULL;
            }
        }
    }

    // notify others
    Writer writer;
    uint8_t type = TYPE_PRESET;
    writer.write(type);
    uint8_t subType = SUBTYPE_CREATE_PRESET;
    writer.write(subType);

    writer.write(_id);
    writer.write(_name);
    size_t ipCount = _ips.size();
    writer.write(ipCount);

    for (ip_set_itr it = _ips.begin(); it != _ips.end(); it++) {
        uint32_t ip = *it;
        writer.write(ip);
    }

    for (ip_set_itr it = _ips.begin(); it != _ips.end(); it++) {
        IPAddress ip(*it);
        NetworkManager.sendTo(writer.getData(), writer.getLength(), ip);
    }
};

void Preset::toJson(JsonObject &root) {
    root["id"] = _id;
    root["n"] = _name;

    JsonArray ipsArray = root.createNestedArray("ips");

    for (ip_set_itr it = _ips.begin(); it != _ips.end(); it++) {
        uint32_t ip = *it;
        IPAddress ipAddr(ip);
        String ipStr = ipAddr.toString();
        ipsArray.add(ipStr);
    }

    root["eA"] = _enableAlexa;
};



Playlist::Playlist(unsigned long id) {
    _id = id;

    _cycleMillis = 10 * 1000;
    _currentIndex = 0;
};

Playlist::Playlist(unsigned long id, String name) {
    _id = id;
    _name = name;

    _cycleMillis = 10 * 1000;
    _currentIndex = 0;
};

unsigned long Playlist::getId() {
    return _id;
};

String Playlist::getName() {
    return _name;
};

void Playlist::load() {
    _lastCycleMillis = millis();
    _currentIndex = 0;

    if (_currentIndex < _presets.size()) {
        PresetManager.loadPreset(_presets[_currentIndex]);
    }
};

void Playlist::update() {
    unsigned long now = millis(); // TODO: maybe synced millis

    if (now - _lastCycleMillis > _cycleMillis) {
        _currentIndex = ++_currentIndex % _presets.size();

        unsigned long presetId = _presets[_currentIndex];

        Serial.print("Playlist ");
        Serial.print(_id);
        Serial.print(" loading preset ");
        Serial.println(presetId);

        PresetManager.loadPreset(presetId);

        _lastCycleMillis = now;
    }
};

void Playlist::fromJson(JsonObject &root) {
    _id = root["id"] | _id;
    _name = root["n"] | _name;

    _cycleMillis = root["cm"] | _cycleMillis;

    JsonArray presetsArray = root["pIds"];
    if (presetsArray) {
        _presets.clear();

        for (JsonVariant idVar : presetsArray) {
            unsigned long id = idVar;
            _presets.push_back(id);
        }
    }
};

void Playlist::toJson(JsonObject &root) {
    root["id"] = _id;
    root["n"] = _name;

    root["cm"] = _cycleMillis;

    JsonArray presetsArray = root.createNestedArray("pIds");

    for (preset_list_itr it = _presets.begin(); it != _presets.end(); it++) {
        unsigned long id = *it;
        presetsArray.add(id);
    }
};







void ESPPresetManager::begin() {
    DynamicJsonDocument doc(5120);
    if (readJson("/presets.json", doc)) {
        JsonObject jsonObj = doc.as<JsonObject>();
        for (JsonPair kv : jsonObj) {
            unsigned long id = strtoul(kv.key().c_str(), NULL, 10);
            Preset *preset = new Preset(id);
            _presets[id] = preset;
        }
    }
};

void ESPPresetManager::savePreset(unsigned long id) {
    DynamicJsonDocument doc(5120);
    readJson("/presets.json", doc);
    JsonObject jsonObj = doc.as<JsonObject>();

    JsonObject configObj = jsonObj.createNestedObject(String(id));
    VirtualDeviceManager.toJson(configObj);

    File f = LittleFS.open("/presets.json", "w");
    serializeJson(jsonObj, f);
    f.close();
};

void ESPPresetManager::loadPreset(unsigned long id, bool notify) {
    DynamicJsonDocument doc(5120);
    if (readJson("/presets.json", doc)) {
        JsonObject jsonObj = doc.as<JsonObject>();
        JsonObject presetObj = jsonObj[String(id)];

        // TODO: maybe fix later - modify for now to keep presets from turning of
        presetObj["on"] = true;

        if (presetObj) {
            VirtualDeviceManager.fromJson(presetObj);
            StorageManager.storeConfig();

            // TODO: test this implementation
            String json;
        	serializeJson(presetObj, json);
        	WebManager.sendToAllClients(json);
        }
    }

    if (notify && _presets.count(id) > 0) {
        Preset *preset = _presets[id];
        preset->load();
    }
};

void ESPPresetManager::deletePreset(unsigned long id) {
    DynamicJsonDocument doc(5120);
    readJson("/presets.json", doc);
    JsonObject jsonObj = doc.as<JsonObject>();

    jsonObj.remove(String(id));

    File f = LittleFS.open("/presets.json", "w");
    serializeJson(jsonObj, f);
    f.close();

    /*
    Preset *preset = _presets[id];
    if (preset) {
        _presets.erase(id);
        delete preset;
    }
    */
}



void ESPPresetManager::savePlaylist(unsigned long id) {
    if (_playlists.count(id) <= 0)
        return;

    Playlist *playlist = _playlists[id];

    DynamicJsonDocument doc(5120);
    readJson("/playlists.json", doc);
    JsonObject jsonObj = doc.as<JsonObject>();

    JsonObject playlistObj = jsonObj.createNestedObject(String(id));
    playlist->toJson(playlistObj);

    File f = LittleFS.open("/playlists.json", "w");
    serializeJson(jsonObj, f);
    f.close();
};

void ESPPresetManager::loadPlaylist(unsigned long id) {
    if (_playlists.count(id) > 0) {
        Playlist *playlist = _playlists[id];
        playlist->load();
    }
};

void ESPPresetManager::deletePlaylist(unsigned long id) {
    DynamicJsonDocument doc(5120);
    readJson("/playlists.json", doc);
    JsonObject jsonObj = doc.as<JsonObject>();

    jsonObj.remove(String(id));

    File f = LittleFS.open("/playlists.json", "w");
    serializeJson(jsonObj, f);
    f.close();

    /*
    if (_playlists.count(id) > 0) {
        Playlist *playlist = _playlists[id];
        if (playlist) {
            _playlists.erase(id);
            delete playlist;
        }
    }
    */
}

void ESPPresetManager::load(unsigned long id, bool notify) {
    if (_playlists.count(id) > 0) {
        _activePlaylistId = id;
        loadPlaylist(id);
    } else if (_presets.count(id) > 0) {
        _activePlaylistId = -1;
        loadPreset(id, notify);
    } else {
        _activePlaylistId = -1;
    }
}

void ESPPresetManager::update() {
    if (_playlists.count(_activePlaylistId) > 0) {
        Playlist *playlist = _playlists[_activePlaylistId];
        if (playlist) {
            playlist->update();
        }
    }
};



bool ESPPresetManager::handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis) {
	uint8_t subType;
	reader.read(&subType);

	if (subType == SUBTYPE_CREATE_PRESET) {
        unsigned long id;
        reader.read(&id);
        String name;
        reader.read(name);
        size_t ipCount;
        reader.read(&ipCount);

        Serial.println("Preset Received: ");
        Serial.print(id);
        Serial.print(": ");
        Serial.print(name);
        Serial.print(" - ");

        ip_set ips;

        for (size_t i = 0; i < ipCount; i++) {
            uint32_t ip;
            reader.read(&ip);

            ips.insert(ip);

            IPAddress ipAddr(ip);
            Serial.print(ipAddr);
            Serial.print(", ");
        }

        Serial.println();

        savePreset(id);
        Preset *preset = new Preset(id, name, ips);
        _presets[id] = preset;
	} else if (subType == SUBTYPE_SET_PRESET) {
        unsigned long id;
        reader.read(&id);

        Serial.print("Remotely set Preset: ");
        Serial.println(id);

        loadPreset(id, false);
	} else {
		return false;
	}

	return true;
}

void ESPPresetManager::fromJson(JsonObject &root) {
    JsonArray prstIds = root["prstIds"];
    long newPrst = root["prstId"];
    JsonArray prsts = root["prsts"];

    if (prstIds) {
        std::vector<unsigned long> create;

        for (JsonVariant prstIdVar : prstIds) {
            unsigned long prstId = prstIdVar.as<unsigned long>();

            if (!_presets.count(prstId)) {
                create.push_back(prstId);
            }
        }

        for (preset_map_itr it = _presets.begin(); it != _presets.end(); ) {
            unsigned long id = it->first;

            bool found = false;

            for (JsonVariant prstIdVar : prstIds) {
                unsigned long prstId = prstIdVar.as<unsigned long>();
                if (id == prstId) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                // delete preset
                deletePreset(id);
                Preset *preset = it->second;
                delete preset;
                _presets.erase(it++);
            } else {
                ++it;
            }
        }

        for (std::vector<unsigned long>::iterator it = create.begin(); it != create.end(); it++) {
            unsigned long id = *it;
            savePreset(id);

            Preset *preset = new Preset(id);
            _presets[id] = preset;
        }
    }

    for (JsonVariant prstVar : prsts) {
        JsonObject prstObj = prstVar.as<JsonObject>();
        unsigned long prstId = prstObj["id"];

        if (_presets.count(prstId)) {
            Preset *prst = _presets[prstId];
            prst->fromJson(prstObj);
        }
    }



    JsonArray pllstIds = root["pllstIds"];
    JsonArray pllsts = root["pllsts"];

    if (pllstIds) {
        std::vector<unsigned long> create;

        for (JsonVariant pllstIdVar : pllstIds) {
            unsigned long pllstId = pllstIdVar.as<unsigned long>();

            if (!_playlists.count(pllstId)) {
                create.push_back(pllstId);
            }
        }

        for (playlist_map_itr it = _playlists.begin(); it != _playlists.end(); ) {
            unsigned long id = it->first;

            bool found = false;

            for (JsonVariant pllstIdVar : pllstIds) {
                unsigned long pllstId = pllstIdVar.as<unsigned long>();
                if (id == pllstId) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                // delete preset
                deletePlaylist(id);

                Playlist *playlist = it->second;
                delete playlist;
                _playlists.erase(it++);
            } else {
                ++it;
            }
        }

        for (std::vector<unsigned long>::iterator it = create.begin(); it != create.end(); it++) {
            unsigned long id = *it;
            savePlaylist(id);

            Playlist *playlist = new Playlist(id);
            _playlists[id] = playlist;
        }
    }

    for (JsonVariant pllstVar : pllsts) {
        JsonObject pllstObj = pllstVar.as<JsonObject>();
        unsigned long pllstId = pllstObj["id"];

        if (_playlists.count(pllstId)) {
            Playlist *pllst = _playlists[pllstId];
            pllst->fromJson(pllstObj);
        }
    }



    if (newPrst) {
        Serial.print("Using Preset ");
        Serial.println(newPrst);
        load(newPrst);
    }
};

void ESPPresetManager::toJson(JsonObject &root) {
    JsonArray presetIdsArray = root.createNestedArray("prstIds");
    JsonArray presetsArray = root.createNestedArray("prsts");

    for (preset_map_itr it = _presets.begin(); it != _presets.end(); it++) {
        unsigned long id = it->first;
        presetIdsArray.add(id);

        Preset *preset = it->second;
        JsonObject presetObj = presetsArray.createNestedObject();
        preset->toJson(presetObj);
    }



    JsonArray playlistIdsArray = root.createNestedArray("pllstIds");
    JsonArray playlistsArray = root.createNestedArray("pllsts");

    for (playlist_map_itr it = _playlists.begin(); it != _playlists.end(); it++) {
        unsigned long id = it->first;
        playlistIdsArray.add(id);

        Playlist *playlist = it->second;
        JsonObject playlistObj = playlistsArray.createNestedObject();
        playlist->toJson(playlistObj);
    }
};

ESPPresetManager PresetManager;

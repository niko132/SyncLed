#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include <Arduino.h>
#include <set>
#include <vector>
#include <map>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

#include "VirtualDeviceManager.h"
#include "Utils.h"
#include "NetworkManager.h"
#include "Alexa.h"

#define SUBTYPE_CREATE_PRESET 1
#define SUBTYPE_SET_PRESET 2

typedef std::set<uint32_t> ip_set;
typedef ip_set::iterator ip_set_itr;

typedef std::vector<unsigned long> preset_list;
typedef preset_list::iterator preset_list_itr;

class Preset {
    private:
        unsigned long _id;
        String _name;
        ip_set _ips;
        bool _enableAlexa;
        EspalexaDevice *_alexaDevice = NULL;

    public:
        Preset(unsigned long id);
        Preset(unsigned long id, String name, ip_set ips);

        unsigned long getId();
        String getName();

        void load();

        void alexaDeviceCallback(EspalexaDevice* d);

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

class Playlist {
    private:
        unsigned long _id;
        String _name;
        unsigned long _cycleMillis;
        unsigned long _lastCycleMillis;
        preset_list _presets;
        unsigned long _currentIndex;

    public:
        Playlist(unsigned long id);
        Playlist(unsigned long id, String name);

        unsigned long getId();
        String getName();

        void load();

        void update();

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

typedef std::map<unsigned long, Preset*> preset_map;
typedef preset_map::iterator preset_map_itr;

typedef std::map<unsigned long, Playlist*> playlist_map;
typedef playlist_map::iterator playlist_map_itr;

class ESPPresetManager {
    private:
        preset_map _presets;
        playlist_map _playlists;

        long _activePlaylistId = -1;

    public:
        void begin();

        void savePreset(unsigned long id);
        void loadPreset(unsigned long id, bool notify=true);
        void deletePreset(unsigned long id);

        void savePlaylist(unsigned long id);
        void loadPlaylist(unsigned long id);
        void deletePlaylist(unsigned long id);

        void load(unsigned long id, bool notify=true);

        bool handlePacket(Reader &reader, IPAddress ip, unsigned long receiveMillis);

        void update();

        void fromJson(JsonObject &root);
        void toJson(JsonObject &root);
};

extern ESPPresetManager PresetManager;

#endif

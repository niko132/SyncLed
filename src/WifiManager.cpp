#include "WifiManager.h"

#include "Utils.h"
#include "WebManager.h"

#include <FS.h>
#include <LittleFS.h>

using namespace std::placeholders;

WifiConfig::WifiConfig() {
    ssid = "";
    password = "";
    ip = IPAddress(0, 0, 0, 0);
    gateway = IPAddress(0, 0, 0, 0);
    subnet = IPAddress(0, 0, 0, 0);
}

bool WifiConfig::hasCredentials() {
    return ssid.length() > 0;
}

bool WifiConfig::hasIp() {
    return IPAddress(0, 0, 0, 0) != ip;
}

void WifiConfig::fromJson(JsonObject &root) {
    ssid = root["ssid"] | ssid;
    password = root["pass"] | password;

    String ipStr = root["ip"] | "0.0.0.0";
    String gatewayStr = root["gateway"] | "0.0.0.0";
    String subnetStr = root["subnet"] | "0.0.0.0";

    ip.fromString(ipStr);
    gateway.fromString(gatewayStr);
    subnet.fromString(subnetStr);
}

void WifiConfig::toJson(JsonObject &root) {
    root["ssid"] = ssid;
    root["pass"] = password;

    root["ip"] = ip.toString();
    root["gateway"] = gateway.toString();
    root["subnet"] = subnet.toString();
}



ESPWifiManager::ESPWifiManager() : _apIP(4, 3, 2, 1) {

}

void ESPWifiManager::autoConnect() {
    LittleFS.begin();

    WifiConfig config = loadConfig();
    if (config.hasCredentials()) {
        Serial.println("Try connecting with credentials");

        if (!connect(config)) {
            Serial.println("Connection failed... starting AP");
            startAP();
        }
    } else {
        Serial.println("No credentials... starting AP");
        startAP();
    }
}

bool ESPWifiManager::connect(WifiConfig config) {
    WiFi.disconnect();

    if (config.hasIp()) {
        WiFi.config(config.ip, config.gateway, config.subnet);
    }

    WiFi.begin(config.ssid, config.password);
    Serial.print("Connecting to: ");
    Serial.println(config.ssid);

    unsigned long startMillis = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startMillis < CONNECT_TIMEOUT_MILLIS) {
        delay(100);
    }

    bool success = WiFi.status() == WL_CONNECTED;
    if (success) {
        Serial.println("Connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.print("Unable to connect to ");
        Serial.println(config.ssid);
    }

    return success;
}

void ESPWifiManager::startAP() {
    Serial.println("Starting AP");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(_apIP, _apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("SyncLED #" + String(ESP.getChipId()), "");

    setupDNS();
    // TODO: dont do double initialization
    WebManager.begin();

    while (true) {
        yield();
        _dnsServer.processNextRequest();
    }
}

void ESPWifiManager::setupDNS() {
    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void ESPWifiManager::onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT) {
        Serial.println("Connect");
    } else if(type == WS_EVT_DISCONNECT) {
        Serial.println("Disconnect");
    } else if(type == WS_EVT_ERROR) {
        Serial.println("Error");
    } else if(type == WS_EVT_PONG) {
        Serial.println("Pong");
    } else if(type == WS_EVT_DATA) {
        DynamicJsonDocument doc(5120);
        DeserializationError err = deserializeJson(doc, data, len);

        if (err) {
            Serial.print("Deserialization failed: ");
            Serial.println(err.c_str());
        } else {
            JsonObject jsonObj = doc.as<JsonObject>();

            String operation = jsonObj["op"] | "";
            if (operation.equals("scan")) {
                Serial.println("Scanning...");
                WiFi.scanNetworksAsync(std::bind(&ESPWifiManager::onScanComplete, this, _1), false);
            } else if (operation.equals("connect")) {
                String ssid = jsonObj["ssid"];
                String pass = jsonObj["pass"];

                Serial.print("Connecting to ");
                Serial.print(ssid);
                Serial.print(" with password ");
                Serial.println(pass);

                WifiConfig config;
                config.ssid = ssid;
                config.password = pass;
                // TODO: add ip stuff
                storeConfig(config);

                ESP.restart();
            }
        }
    }
}

void ESPWifiManager::onScanComplete(int numNetworks) {
    Serial.print("Scan complete: ");
    Serial.println(numNetworks);

    DynamicJsonDocument doc(5120);
    JsonArray jsonArray = doc.to<JsonArray>();

    for (int i = 0; i < numNetworks; i++) {
        JsonObject networkObj = jsonArray.createNestedObject();

        networkObj["ssid"] = WiFi.SSID(i);
        networkObj["rssi"] = WiFi.RSSI(i);
        networkObj["enc"] = WiFi.encryptionType(i);
    }

    String json;
    serializeJson(doc, json);
    WebManager.sendToAllWifiConfigClients(json);
}

WifiConfig ESPWifiManager::loadConfig() {
    WifiConfig config;

    DynamicJsonDocument doc(5120);
    if (readJson("/wifi_config.json", doc)) {
        JsonObject configObj = doc.as<JsonObject>();
        config.fromJson(configObj);
    }

    return config;
}

void ESPWifiManager::storeConfig(WifiConfig config) {
    DynamicJsonDocument doc(5120);
    JsonObject jsonObj = doc.to<JsonObject>();

    config.toJson(jsonObj);

    File f = LittleFS.open("/wifi_config.json", "w");
    serializeJson(jsonObj, f);
    f.close();
}

ESPWifiManager WifiManager;

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#define DEFAULT_CONFIG "{\"vdIds\":[1],\"vds\":[{\"id\":1,\"sI\":0,\"eI\":180,\"eId\":1,\"pId\":3,\"eD\":{\"d\":5000,\"sP\":0.0,\"eP\":1.0}}],\"pIds\":[3],\"ps\":[{\"id\":3,\"n\":\"RedAndGreen\",\"ks\":[{\"p\":0.0,\"c\":255},{\"p\":0.5,\"c\":65280}]}]}"

class ESPStorageManager {
    private:
        bool _storeFlag = false;

    public:
        void begin();
        void update();

        void loadConfig();
        void storeConfig();
};

extern ESPStorageManager StorageManager;

#endif

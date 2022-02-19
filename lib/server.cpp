#include<BLEServer.h>

class ServerInspector : public BLEServerCallbacks {
    public:
    virtual ~ServerInspector() {};
    virtual void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {

    }

    virtual void onDisconnect(BLEServer* pServer) {

    }
};

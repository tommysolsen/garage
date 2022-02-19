#include <Arduino.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <string>

#include "utils.h"

#define SERIAL_BAUD 115200

#define SERVICE_UUID "5eed4b32-91cf-4e6d-9f59-500a217f4fd2"
#define STATUS_CHARACTERISTIC_UUID "641fa9d3-4001-46f2-a25d-d716a6c738b1"
#define READ_NONCE_CHARACTERISTIC_UUID "25f35bdc-eff4-4f6e-b0a9-68c1b0559cc0"
#define RESET_NONCE_CHARACTERISTIC_UUID "25f35bdc-eff4-4f6e-b0a9-68c1b0559cc1"
#define OPEN_CHARACTERISTIC_UUID "cd3903a1-75ed-4630-818c-5b13a0da9df3"

bool connected = false;
bool ledOn = false;

bool canOpen = true;

class ServerInspector : public BLEServerCallbacks
{
public:
    virtual ~ServerInspector(){};
    virtual void onConnect(BLEServer *pServer, esp_ble_gatts_cb_param_t *param)
    {
        Serial.printf("Device connected: %s\r\n", "yay");
        connected = true;
    }

    virtual void onDisconnect(BLEServer *pServer)
    {
        Serial.printf("Device disconnected\r\n");
        connected = false;
    }
};

class GarageCharacteristic : public BLECharacteristicCallbacks
{

private:
    BLECharacteristic *read;
    BLECharacteristic *nonceReader;
    BLECharacteristic *nonceSetter;

    BLECharacteristic *open;

    bool canOpen;
    std::string nonce;

    void setCanOpen(bool value)
    {
        uint32_t p = value ? 0x00000001 : 0x00000000;
        this->read->setValue(p);
        this->canOpen = value;
    }

    bool getCanOpen()
    {
        return this->canOpen;
    }

    void setNonce(std::string nonce)
    {
        this->nonce = nonce;
        this->nonceReader->setValue(nonce);
    }

public:
    GarageCharacteristic(BLEService *service)
    {
        this->read = service->createCharacteristic(
            STATUS_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ);

        this->nonceReader = service->createCharacteristic(
            READ_NONCE_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

        this->nonceSetter = service->createCharacteristic(
            RESET_NONCE_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE);

        this->open = service->createCharacteristic(
            OPEN_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE);

        this->nonceReader->addDescriptor(new BLE2902());
        this->read->setCallbacks(this);
        this->nonceReader->setCallbacks(this);
        this->nonceSetter->setCallbacks(this);
        this->open->setCallbacks(this);

        this->setNonce("");
        this->setCanOpen(false);
    }
    virtual ~GarageCharacteristic(){};

    void resetNonce()
    {
        char str[100];
        sprintf(str, "kake-%d", rand() % 9);
        this->setNonce((std::string)str);
    }

    void broadcast()
    {
        this->nonceReader->notify();
    }

    virtual void onWrite(BLECharacteristic *pCharacteristic)
    {
        if (pCharacteristic == this->nonceReader)
        {
            this->canOpen = true;
            this->resetNonce();
        }
        else if (pCharacteristic == this->open)
        {
            if (this->canOpen == false)
            {
                return;
            }
            Serial.printf("Writing to characteristic: %s\r\n", pCharacteristic->getData());

            std::string data(reinterpret_cast<char *>(pCharacteristic->getData()));

            uint32_t p = 0x00000000;
            if (strcmp(data.c_str(), this->nonce.c_str()) == 0)
            {
                toggleGarage();
                this->setNonce("");
                this->setCanOpen(false);
                p = 0x00000001;
            }
            else
            {
                signalAuthenticationFailure();
            }
            pCharacteristic->setValue(p);
        }
    }
};

GarageCharacteristic *c;

void setup()
{
    // Set
    pinMode(POWER_LED, OUTPUT);
    pinMode(GARAGE_PORT, OUTPUT);

    Serial.begin(SERIAL_BAUD);

    BLEDevice::init("Garage");

    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new ServerInspector());
    BLEService *service = server->createService(SERVICE_UUID);

    c = new GarageCharacteristic(service);
    service->start();

    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    enableManualOpenClose();

    Serial.printf("BLE Server ready: %s\r\n", SERVICE_UUID);
    initiatedPing(POWER_LED);
    digitalWrite(POWER_LED, LOW);
}

int i = 0;
void loop()
{
    if (connected && !ledOn)
    {
        ledOn = connected;
        digitalWrite(POWER_LED, HIGH);
    }
    else if (!connected && ledOn)
    {
        ledOn = connected;
        digitalWrite(POWER_LED, LOW);
    }

    if (connected)
    {
        c->broadcast();
    }

    if (i == 10)
    {
        rand();
        i = 0;
    }
    else
    {
        i++;
        delay(100);
    }
}

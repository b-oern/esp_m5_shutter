/* 
 * Sample program for ESP32 acting as a Bluetooth keyboard
 * Sends VOLUME_UP (0x81) when button on IO37 is pressed
 */

#define LED_PIN 19

#define US_KEYBOARD 1

#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

// Change the below values if desired
#define BUTTON_PIN 37
#define DEVICE_NAME "ESP32 Keyboard"

// HID Consumer Control key code for Volume Up
#define VOLUME_UP 0x81

// Forward declarations
void bluetoothTask(void*);
void sendConsumerKey(uint16_t key);

bool isBleConnected = false;

void setup() {
    Serial.begin(115200);

    // configure pin for button
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // HIGH = off (active low)

    // start Bluetooth task
    xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
}

void loop() {  
    if (isBleConnected && digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("Button pressed: sending VOLUME_UP");
        sendConsumerKey(VOLUME_UP);
        delay(200); // debounce
    }

    delay(100);
}

// HID Consumer Control report (2 bytes: key code)
struct ConsumerReport {
    uint16_t key;
};

// Report map for Consumer Control device (Volume Up/Down etc.)
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1),      0x0C,        // Consumer Devices
    USAGE(1),           0x01,        // Consumer Control
    COLLECTION(1),      0x01,        // Application
    REPORT_ID(1),       0x02,        //   Report ID (2)
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(2),   0xFF, 0x03,  //   Usage Maximum (0x03FF)
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(2), 0xFF, 0x03,  //   Logical Maximum (0x03FF)
    REPORT_COUNT(1),    0x01,        //   1 value
    REPORT_SIZE(1),     0x10,        //   16 bits
    HIDINPUT(1),        0x00,        //   Data, Array, Abs
    END_COLLECTION(0)
};

BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

class BleKeyboardCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* server) {
        isBleConnected = true;
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(true);
        Serial.println("Client has connected");
    }

    void onDisconnect(BLEServer* server) {
        isBleConnected = false;
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(false);
        Serial.println("Client has disconnected");
    }
};

void bluetoothTask(void*) {
    BLEDevice::init(DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());

    hid = new BLEHIDDevice(server);
    input = hid->inputReport(2); // report ID 2 (Consumer Control)

    hid->manufacturer()->setValue("Maker Community");
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    hid->hidInfo(0x00, 0x02);

    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);

    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();
    hid->setBatteryLevel(100);

    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->addServiceUUID(hid->deviceInfo()->getUUID());
    advertising->addServiceUUID(hid->batteryService()->getUUID());
    advertising->start();

    Serial.println("BLE ready");
    delay(portMAX_DELAY);
}

void sendConsumerKey(uint16_t key) {
    // Send key press
    ConsumerReport report = { .key = key };
    input->setValue((uint8_t*)&report, sizeof(report));
    input->notify();
    delay(30);

    // Send key release (key = 0)
    ConsumerReport release = { .key = 0x00 };
    input->setValue((uint8_t*)&release, sizeof(release));
    input->notify();
    delay(10);
}
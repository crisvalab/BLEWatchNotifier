#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config.h"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
//uint32_t value = 0;

TTGOClass *ttgo;

TFT_eSPI *tft =  nullptr;
bool irq = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void display_show()
{
    ttgo->openBL();
    tft->fillScreen(TFT_BLACK);
    tft->drawString(String(ttgo->power->getBattPercentage()) + " %", 25, 100);
    //tft->println(" %");
}

void setup() {
  ttgo = TTGOClass::getWatch();
  ttgo->begin();

  tft = ttgo->tft;

  tft->setTextFont(2);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);

  pinMode(AXP202_INT, INPUT_PULLUP);
  attachInterrupt(AXP202_INT, [] {
      irq = true;
  }, FALLING);

  ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);
  ttgo->power->clearIRQ();

  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  
  //pAdvertising->setMinPreferred(0x0);
  //pAdvertising->setMaxPreferred(0x0);

  //pAdvertising->setMaxInterval(0x460);
  //pAdvertising->setMinInterval(0x460);
  
  BLEDevice::startAdvertising();
}

void loop() {
    if (irq) {
        irq = false;
        ttgo->power->readIRQ();
        if (ttgo->power->isPEKShortPressIRQ()) {
            display_show();
            delay(2000);
            ttgo->closeBL();
        }
        ttgo->power->clearIRQ();
    }
    
    //lv_task_handler();
    delay(5);
}

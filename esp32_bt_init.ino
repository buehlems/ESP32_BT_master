//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Victor Tchistiak - 2019
//
//This example demonstrates reading and removing paired devices stored on the ESP32 flash memory
//Sometimes you may find your ESP32 device could not connect to the remote device despite
//many successful connections earlier. This is most likely a result of client replacing your paired
//device info with new one from other device. The BT clients store connection info for paired devices,
//but it is limited to a few devices only. When new device pairs and number of stored devices is exceeded,
//one of the previously paired devices would be replaced with new one.
//The only remedy is to delete this saved bound device from your device flash memory
//and pair with the other device again.
//
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include"esp_gap_bt_api.h"
#include "esp_err.h"
#include <Streaming.h>

#define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove

#define PAIR_MAX_DEVICES 20
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];


bool setBtPowerLevel(esp_power_level_t pmin, esp_power_level_t pmax=ESP_PWR_LVL_N12){
  // https://stackoverflow.com/questions/64153279/max-tx-power-classic-bluetooth-for-esp32
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/controller_vhci.html
  if(pmax<pmin)
    pmax=pmin;
  esp_power_level_t min,max; 
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV,ESP_PWR_LVL_P9 ); 
  // esp_bredr_tx_power_set(ESP_PWR_LVL_P9,ESP_PWR_LVL_P9  ); 
  esp_bredr_tx_power_set(pmin,pmax ); // default is N0=4/P3=5
  delay(1000); 
  esp_bredr_tx_power_get(&min,&max); 
  Serial << "setBtPowerLevel: desired min/max=" << pmin << "/" << pmax << " actual min/max=" << min << "/" << max << endl;
  // Serial.printf("min %d max %d",min,max); 
  return true;
}

bool initBluetooth()
{
  // esp_bt_controller_init(&cfg) & esp_bt_controller_enable(BT_MODE)
  if(!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }
 
  if(esp_bluedroid_init() != ESP_OK) {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }
 
  if(esp_bluedroid_enable() != ESP_OK) {
    Serial.println("Failed to enable bluedroid");
    return false;
  }
  return true;
}

char *bda2str(const uint8_t* bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}
 
void remove_devs()
{
  char bdabuf[20];
  Serial.print("ESP32 bluetooth address: "); Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
  // Get the numbers of bonded/paired devices in the BT module
  int count = esp_bt_gap_get_bond_device_num();
  if(!count) {
    Serial.println("No bonded device found.");
  } else {
    Serial.print("Bonded device count: "); Serial.println(count);
    if(PAIR_MAX_DEVICES < count) {
      count = PAIR_MAX_DEVICES; 
      Serial.print("Reset bonded device count: "); Serial.println(count);
    }
    esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if(ESP_OK == tError) {
      for(int i = 0; i < count; i++) {
        Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
        Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));     
        if(REMOVE_BONDED_DEVICES) {
	  Serial << "removing device #" << i << "addr=" << bda2str(pairedDeviceBtAddr[i],bdabuf,20) << endl;
          esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
          if(ESP_OK == tError) {
            Serial.print("Removed bonded device # "); 
          } else {
            Serial.print("Failed to remove bonded device # ");
          }
          Serial.println(i);
        }
      }        
    }
  }
}
 
//void loop() {}

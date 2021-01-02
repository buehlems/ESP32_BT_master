//This example code is in the Public Domain (or CC0 licensed, at your option.)
//Original code by Victor Tchistiak - 2019
//Improved by Markus Buehler - 2021
//
//This example demostrates master mode bluetooth connection and pin 
//it creates a bridge between Serial and Classical Bluetooth (SPP)
//this is an extention of the SerialToSerialBT example by Evandro Copercini - 2018
//

#include <Streaming.h>

#include "esp_bt.h"
// requires ESP lib 1.0.2 and uses fix from https://github.com/espressif/arduino-esp32/issues/3916#issuecomment-625486992
#include "BluetoothSerial_Fix.h"


BluetoothSerial SerialBT;

String MACadd = "AA:BB:CC:11:22:33";
uint8_t address[6]  = {0x20, 0x16, 0x01, 0x18, 0x39, 0x62}; // HC-05
// uint8_t address[6]  = {0x00, 0x80, 0x25, 0x08, 0xC3, 0x2A}; // SMA
String name = "bt_arduino";
char *pin = "0000"; //<- standard pin would be provided by default
bool connected=false;
int trials=0;

bool setBtPowerLevel(esp_power_level_t pmin, esp_power_level_t pmax);

int bt_connect(uint8_t *addr, int maxtrials=10){
  int trials=-1;
  for(trials=1; trials <= maxtrials; trials++){
    Serial << "trying to connect #" << trials << "\n";
    if(addr)
      connected = SerialBT.connect(address);
    else
      connected = SerialBT.connect();
    if(connected)
      break;
  }
  return trials <= maxtrials ? trials:-1;
}

int bt_reconnect(int maxtrials=10){
  return bt_connect(NULL,maxtrials);
}

void setup() {
  char bdabuf[20];
  Serial.begin(115200);

  Serial << "initBluetooth\n";
  initBluetooth();
  setBtPowerLevel(ESP_PWR_LVL_P9,ESP_PWR_LVL_P9);
  //Remove all previously bonded device...
  //... to ensure we are starting with a "clean slate"

  Serial << "remove_devs\n";
  remove_devs();    //See Tab

  // SerialBT.enableSSP();
  Serial << "start bluetooth ESP32test\n";
  SerialBT.begin("ESP32test", true); 
  Serial << "set pin " << pin << endl;
  SerialBT.setPin(pin);
  Serial << "The device started in master mode, make sure remote BT device is on: " << bda2str(address,bdabuf,18) << endl;
  
  // connect(address) is fast (upto 10 secs max), connect(name) is slow (upto 30 secs max) as it needs
  // to resolve name to address first, but it allows to connect to different devices with the same name.
  // Set CoreDebugLevel to Info to view devices bluetooth address and device names
  // connected = SerialBT.connect(name);
  // connected = SerialBT.connect(address);

  trials=bt_connect(address,10);

  if(trials<0){
    Serial << "Failed to connect. Make sure remote device is available and in range, then restart app." << endl;
    while(1)
      sleep(1);
  }

  Serial << "Connected successfully after " << trials << " trials\n";

/*   // disconnect() may take upto 10 secs max */
/*   if (SerialBT.disconnect()) { */
/*     Serial.println("Disconnected Succesfully!"); */
/*   } */
/*   // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address). */
/*   SerialBT.connect(); */
}

unsigned long last_disconnect=0;
unsigned long seconds=0;

void loop() {
  int trials;
  unsigned long time;

  if(millis()>last_disconnect+10000){
    Serial << "disconnecting ... ";
    if (SerialBT.disconnect()) {
      Serial << "ok" << endl;
    }else{
      Serial << "fail" << endl;
    }
    Serial << "reconnecting ..." << endl;
    trials=bt_reconnect(10);
    // trials=bt_connect(address,10);
    if(trials<0){
      Serial << "Failed to connect. Make sure remote device is available and in range, then restart app or reset." << endl;
    }else{
      Serial << "Connected successfully after " << trials << " trials\n";
    }
    last_disconnect=millis();
    seconds=0;
  }

  time=(millis()-last_disconnect)/1000;
  if(time > seconds){
    seconds=time;
    Serial << " .." << seconds << "s" ".. " << endl;
  }  

  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
}

# Connect an ESP32-master with HC-05 Slave
## Hardware
### ESP32
ESP32 UNO D1 R3 R32 WIFI Bluetooth USB-B CH340  
https://www.ebay.de/itm/273605232455?ul_noapp=true

### HC-05 + Arduino Mega 2650
ITEAD Wireless Bluetooth Shield Module Starter Kit For Arduino  
Model: IM120417010  
https://www.itead.cc/itead-bt-shield-master-slave.html

## Setup the HC-05
- On the Arduino load a simple blink program to be sure that the Arduino doesn't do anything bad.  
- Connect jumper on Bluetooth:  
D0-RX  
D1-TX  
- Switch HC-05 into command mode.
- Setup the HC-05 with a terminal program (e.g. Teraterm)  
  - 38400 Bd  
  - Receive: CR  
  - Transmit: CR+LF  
- Setup the HC-05  

```
at+orgl # reset to manufacturer settings
OK
at+cmode=1 # connect to any address
OK
at+adcn # number of connected devices
+ADCN:1
OK
at+rmaad # remove any connected devices
OK
at+adcn
+ADCN:0
OK
at+rname?246f,28,975112 # get name of device with addr 24:6f:28:97:51:12
+RNAME:ESP32test # this is my ESP32
OK
at+pswd=0000 # set the pin
OK
```
### HC-05 Shield Blink Codes
0.5 HZ: CMD mode (disconnected, not paired)  
2 Hz: waiting to pair  
0.5 Hz, short blink: paired  
blink, blink, pause, blink, blink, pause: connected  
## ESP32test
at the time of development ESP library 1.0.4 was the most recent. This version has a bug in BT_Serial as described in
https://github.com/espressif/arduino-esp32/issues/3916#issuecomment-625486992
#### Summary of the post
- waitForConnect() function will always return
- the BT master mode does not work anymore as implemented in 1.0.2  

**Solution:**
- switch to version 1.0.2
- replace [Bluetooth_Serial.cpp and Bluetooth_Serial.h with patch](https://github.com/espressif/arduino-esp32/blob/f0e2e2a62fe98ec9657f3935c41c88a0fd0e7acd/libraries/BluetoothSerial/src). I copied the two files to a local library and renamed it to `BluetoothSerial_Fix`.

Remark: if you patch BluetoothSerial but stay with 1.0.4 you'll get the `Stack smashing protect failure!` error as mentioned in one of the other posts.

 ### BT_Master
 This code has been stiched together from the standard [SerialToSerialBTM](https://github.com/espressif/arduino-esp32/blob/master/libraries/BluetoothSerial/examples/SerialToSerialBTM/SerialToSerialBTM.ino) and [mprowe's](https://github.com/mprowe )[odb2_ESP32_BT.zip](https://github.com/espressif/arduino-esp32/files/4501632/odb2_ESP32_BT.zip)  
 From the latter I got the functions in `esp32_bt_init`. Tx for that.  
 You will need to set `uint8_t address[6]` to the address of your HC-05.
 The code does:  

**setup()**
 - initialize BT classic
 - set the BT power level to its maximum. But even with that the two HC-05 and the ESP32 have to be very close (20 cm)
 - initially connect to the HC-05  

**loop()**
- receive and send characters from/to BT
- print the received characters
- every 10s: disconnect and reconnect BT (this is for experimental purposes, to test how stable reconnection works)

### Typical Ouput
```
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0018,len:4
load:0x3fff001c,len:1100
load:0x40078000,len:9232
load:0x40080400,len:6400
entry 0x400806a8
initBluetooth
setBtPowerLevel: desired min/max=7/7 actual min/max=7/7
remove_devs
ESP32 bluetooth address: 24:6f:28:97:51:12
Bonded device count: 1
Found bonded device # 0 -> 20:16:01:18:39:62
removing device #0addr=20:16:01:18:39:62
Removed bonded device # 0
start bluetooth ESP32test
set pin 0000
The device started in master mode, make sure remote BT device is on: 20:16:01:18:39:62
trying to connect #1
Connected successfully after 1 trials
 ..3s..
 ..4s..
 ..5s..
 ..6s..
 ..7s..
 ..8s..
 ..9s..
disconnecting ... ok
reconnecting ...
trying to connect #1
trying to connect #2
Connected successfully after 2 trials
 ..1s..
 ..2s..
 ..3s..
 ..4s..
 ..5s..
Messa ..6s..
ge
 ..7s..
 ..8s..

```

## General Remark
A found that the range at least of my ESP32 device is very disappointing. This is true not only for BT but for WiFi also:  
**BT**: with the HC-05 I am able to connect to another BT device which is one floor below, seperated by reinforced-concrete. With the ESP32 the distance between the two devices must be not more than 0.5m or so.  
**WiFi**: I had used a TLG10UA03 before. With that I am able to connect to my router which is placed in an adjacent room. The ESP32 can only connect in the same room.  

This is very disappointing. I want to read out my solar inverter by bluetooth and post the data on a website. This works fine with my HC-06/TLG10UA03 combination. I had planned to replace this by a single ESP32, since it seems to combine all the features I need + attractive low power features so that I would be able to power this device by a cheap solar module.  
Any hints how to increase the WiFi and BT range of the ESP32 are highly welcome.

# ESP32WaterMeter

ESP32 project that integrates with Domoticz a Water Flow Meter Using a proximity sensor and a cheap water meter

## Hardware used

1. AZDelivery ESP32 ESP-WROOM-32 NodeMCU  WiFi Module
2. AZDelivery KY-016 FZ0455 3-Color RGB LED Module 
3. GUMEI Proximity Sensor 5V  (https://www.amazon.es/gp/product/B09BZ4FCF8/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1)
4. Water meter (https://www.climabit.com/contadores/2857-contador-agua-fria-zenner-etkd-m-r160-dn15-mm-34-34.html))

## ESP32 Connection Diagram

1. AZDelivery KY-016 FZ0455 3-Color RGB LED Module is connected to PINS 25 (BLUE) 26 (GREEN) and 27 (RED) + power
2. GUMEI Proximity Sensor is connected to PIN 34 + power

## Domoticz setup

Define a virtual sensor (type Counter Incremental). In order to count Liters, Counter Divider needs to be changed to 1000. Virtual Device IDX must match variable DeviceID in the ESP32 program.

## Configuration

1. Before flashing, variables ssid and password must be changed so that ESP32 module can connect to your wifi.

## Running the programs

Once flashed and with the ESP32 module connected to your Wifi finish the configuration in http://yourESP32IP

1. Change Device ID with the ID of your Domoticz virtual sensor 
2. Change Domoticz User, Password and Server

Unfortunatly if the ESP32 device is rebooted all this configuration is lost. I recommend to do this configuration changes in source code before flashsing!





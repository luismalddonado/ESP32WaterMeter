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

## Configuration - Before Flashing

1. Change variables ssid and password so that ESP32 module can connect to your Wifi.
2. Change variables domoticzDevice, domoticzuser, domoticzpass and domoticzserver  so that ESP32 module can connect to your Domoticz Server.

## Running the programs

Once flashed and started, the program:

1. Connects to your Wifi
2. Connects to Domoticz server
3. Counts proximity sensor changes every 0.5 segs and updates Domoticz every minute, if changes in proximity sender reads.
4. LED values: RED if no connectivity to Domoticz or Wifi. BLUE if connected to Domoticz and proximity sensor is active. GREEN if connected to Domoticz and proximity sensor not active





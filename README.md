# ESP-NOW Receiver
This is a project that uses an ESP32 board as a receiver for ESP-NOW messages sent by other ESP boards and passes the data onto serial, either via USB or UART.  
Mostly as a fun project to avoid wifi config on ESP32 sensors.  

## How to connect ESP32 to Raspberry PI GPIOs:
Connect ESP32 RX to GPIO4 (UART3 TX), and TX to GPIO5 (UART3 RX)  
[Example pin map for Seeed Studios tiny ESP32 boards](images/pin-map.png)

Add the following line to the end of config.txt:
> dtoverlay=uart3

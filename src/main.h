#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <HardwareSerial.h>

#define RESPONSE_MAXDELAY 400
#define INCOMING_QUEUE_SIZE 100

#define ACK 0x06

typedef struct {
    bool handled;
} struct_response;

typedef struct {
    uint8_t mac[ESP_NOW_ETH_ALEN];
    String message;
} incoming_message_event;
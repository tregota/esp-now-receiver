#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define REPROGRAMMING_DELAY 5    /* Time to wait for reprogramming (in seconds) (will be subtracted from TIME_TO_SLEEP)*/

#define BUTTON_PIN D1
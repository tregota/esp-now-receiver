#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define uS_TO_S_FACTOR 1000000   /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  300       /* Time to deep sleep (in seconds) */
#define REPROGRAMMING_DELAY 5    /* Time to wait for reprogramming (in seconds) (will be subtracted from TIME_TO_SLEEP)*/
#define LIGHT_LEVEL_MAXIMUM 4095

#define BATTERY_PIN A0
#define TEMPERATURE_PIN A1
#define LIGHT_LEVEL_PIN A2
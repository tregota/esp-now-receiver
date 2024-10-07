#include "main.h"

OneWire tempOneWire(TEMPERATURE_PIN);
DallasTemperature DS18B20(&tempOneWire);
String jsonstring;
JsonDocument jsondoc;

const char espnow_id[] = "SensorTemp"; // sender id
const uint8_t receiver_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // replace with mac address of receiver to not broadcast

RTC_DATA_ATTR bool first_run = true; // RTC_DATA_ATTR is used to keep the value of first_run between deep sleep cycles

bool espnow_message_sent = false;

void on_send(const uint8_t *mac_addr, esp_now_send_status_t status) {
	espnow_message_sent = true;
}

void setup() {

  /////// REPROGRAMMING ///////

  if (first_run) {
    Serial.begin(115200);
    delay(REPROGRAMMING_DELAY * 1000); 
  }

  /////// GATHER DATA ///////

  DS18B20.begin(); // initiate the DS18B20 sensor
  DS18B20.requestTemperatures(); // request the temperature from the DS18B20 sensor
  float temperature = DS18B20.getTempCByIndex(0); // get the temperature in celsius
  
  pinMode(LIGHT_LEVEL_PIN, INPUT);  // declare the LIGHT_LEVEL_PIN as an INPUT
  float lightLevel = (LIGHT_LEVEL_MAXIMUM - analogRead(LIGHT_LEVEL_PIN)) / (LIGHT_LEVEL_MAXIMUM / 100.0); // convert the value to a light level between 0 and 100

  jsondoc["id"] = espnow_id;
  jsondoc["temperature"] = (int)(temperature * 100 + 0.5) / 100.0; // rounds to 2 decimal places
  jsondoc["lightlevel"] = (int)(lightLevel * 100 + 0.5) / 100.0;
  serializeJson(jsondoc, jsonstring);

  /////// SEND DATA ///////

  if (jsonstring.length() <= ESP_NOW_MAX_DATA_LEN) {
    WiFi.mode(WIFI_STA);
    esp_wifi_set_protocol(WIFI_IF_STA , WIFI_PROTOCOL_LR);
    esp_now_init();

    esp_now_register_send_cb(on_send);

    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, receiver_address, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      if (first_run) {
        Serial.printf("Sending: %s\n", jsonstring.c_str());
      }
      if(esp_now_send(receiver_address, (uint8_t *) jsonstring.c_str(), jsonstring.length()) == ESP_NOW_SEND_SUCCESS) {
        while (!espnow_message_sent){
          delay(1); // if we dont wait, the message might not be sent before going to sleep
        }
      }
      else if (first_run) {
        Serial.println("Failed to send message");
      }
    }
    else if (first_run) {
      Serial.println("Failed to add peer");
    }
  }
  else if (first_run) {
    Serial.println("Message too long, skipping transmission..");
  }

  /////// SLEEP ///////

  if (first_run) {
    first_run = false;
  }

  esp_sleep_enable_timer_wakeup((TIME_TO_SLEEP * uS_TO_S_FACTOR) - micros());
  esp_deep_sleep_start();
}

void loop() {
}
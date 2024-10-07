#include "main.h"

String jsonstring;
JsonDocument jsondoc;

const char espnow_id[] = "ButtonTest"; // sender id
const uint8_t receiver_address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // replace with mac address of receiver to not broadcast

bool espnow_message_sent = false;

void on_send(const uint8_t *mac_addr, esp_now_send_status_t status) {
	espnow_message_sent = true;
}

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  Serial.begin(115200);

  /////// REPROGRAMMING ///////

  if (digitalRead(BUTTON_PIN) == LOW) {

    /////// GATHER DATA ///////

    jsondoc["id"] = espnow_id;
    jsondoc["button"] = "pressed";
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
        if(esp_now_send(receiver_address, (uint8_t *) jsonstring.c_str(), jsonstring.length()) == ESP_NOW_SEND_SUCCESS) {
          while (!espnow_message_sent){
            delay(1); // if we dont wait, the message might not be sent before going to sleep
          }
        }
        else {
          Serial.println("Failed to send message");
        }
      }
      else {
        Serial.println("Failed to add peer");
      }
    }
    else {
      Serial.println("Message too long, skipping transmission..");
    }

    /////// WAIT FOR RELEASE ///////

    WiFi.mode(WIFI_OFF);
    setCpuFrequencyMhz(10);
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
    }
  }

  /////// SLEEP ///////

  esp_deep_sleep_enable_gpio_wakeup(BIT(BUTTON_PIN), ESP_GPIO_WAKEUP_GPIO_LOW);
  esp_deep_sleep_start();
}

void loop() {
}
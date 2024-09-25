#include "main.h"

String jsonstring;
JsonDocument jsondoc;
struct_response data_response;

#define ESPNOW_ID "doorbell" // button id
uint8_t receiver_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // replace with mac address of receiver

bool espnow_answer_received = false;

int battery_percentage[] = {4200, 4150, 4110, 4080, 4020, 3980, 3950, 3910, 3870, 3850, 3840, 3820, 3800, 3790, 3770, 3750, 3730, 3710, 3690, 3610, 3270};
int get_battery_percentage(){
  int battery_mv = get_battery_voltage();
  int perc = 0;
  for(int i=0; i<=20; i++) if(battery_mv > battery_percentage[20-i]) perc+=5;
  return constrain(perc, 0, 100);
}

void on_data_recv(const uint8_t * mac, const uint8_t *data, int len) {
  memcpy(&data_response, data, sizeof(data_response));
  espnow_answer_received = true;
}

void setup(){
  pinMode(BUTTON_PIN, INPUT);
  pinMode(ADC_ENABLE_PIN, OUTPUT);
  pinMode(ADC_PIN, INPUT);
  analogReadResolution(12);
  digitalWrite(ADC_ENABLE_PIN, HIGH);

  btStop();
  WiFi.mode(WIFI_STA);
  if (esp_wifi_set_protocol(WIFI_IF_STA , WIFI_PROTOCOL_LR) != ESP_OK) {
    printf("Error ESP-NOW protocol set failed\r\n");
    return;
  }

  FastLED.addLeds<APA102, APA102_SDI_PIN, APA102_CLK_PIN, BGR>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.setBrightness(180);
  delay(50);

  if (esp_now_init() != ESP_OK) {
    printf("Error initializing ESP-NOW\r\n");
    return;
  }
  
  esp_now_peer_info_t peerInfo;

  memcpy(peerInfo.peer_addr, receiver_address, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
      
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    set_fastled(CRGB::Red, CRGB::Black);
    delay(1500);
  }
  else {
    esp_now_register_recv_cb(on_data_recv);

    // Fill json doc with values and serialize to String
    jsondoc["id"] = ESPNOW_ID;
    jsondoc["type"] = "button";
    jsondoc["event"] = "button_pressed";
    jsondoc["batterylevel"] = get_battery_percentage();
    serializeJson(jsondoc, jsonstring);

    set_fastled(CRGB::MediumBlue);
    esp_now_send(NULL, (uint8_t *) jsonstring.c_str(), jsonstring.length());

    // wait on espnow answer
    unsigned long t_wait_answer_start = millis();
    while (!espnow_answer_received && millis() <= t_wait_answer_start + 100){
      delayMicroseconds(1);
    }

    // This will reduce power consumption.
    WiFi.mode(WIFI_OFF);
    setCpuFrequencyMhz(10);

    if (!espnow_answer_received || !data_response.handled) {
      set_fastled(CRGB::Red);
    }
  }
  delay(1000);
  
  // let user keep device awake for max 1 second, by holding the button, to allow for new software to be flashed
  unsigned long t_wait_for_new_software_start = millis();
  while (digitalRead(BUTTON_PIN) == 1 && millis() <= t_wait_for_new_software_start + 1000){
    delay(50);
  }

  set_fastled(CRGB::Black);

  esp_deep_sleep_start();
}

void loop() {
  
}
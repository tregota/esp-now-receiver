#include "main.h"

// uart serial
HardwareSerial UartSerial(0);

// event queue for serial com and espnow responses
static QueueHandle_t messageQueue;

void esp_now_receive_callback(const uint8_t* mac, const uint8_t* data, int len) {
  // queue message for processing
  incoming_message_event newEvent;
  memcpy(&newEvent.mac, mac, sizeof(newEvent.mac));
  newEvent.message = String((char*) data, len); // convert data to string
  if (xQueueSend(messageQueue, &newEvent, RESPONSE_MAXDELAY) != pdTRUE) {
    UartSerial.printf("ERROR:Add to queue failed\r\n");
    Serial.println("ERROR:Add to queue failed");
  }
}

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  UartSerial.begin(115200);


  WiFi.mode(WIFI_STA);
  
  if (esp_wifi_set_protocol(WIFI_IF_STA , WIFI_PROTOCOL_LR) != ESP_OK) {
    UartSerial.println("ERROR:ESP-NOW protocol set failed");
    Serial.println("ERROR:ESP-NOW protocol set failed");
    return;
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    UartSerial.println("ERROR:ESP-NOW initialization failed");
    Serial.println("ERROR:ESP-NOW initialization failed");
    return;
  }

  messageQueue = xQueueCreate(INCOMING_QUEUE_SIZE, sizeof(incoming_message_event));
  if (messageQueue == NULL) {
    UartSerial.println("ERROR:Create mutex fail");
    Serial.println("ERROR:Create mutex fail");
    return;
  }
  // Register the ESP-NOW receiver callback
  esp_now_register_recv_cb(esp_now_receive_callback);
}

esp_now_peer_info_t respondPeerInfo;
incoming_message_event event;
struct_response data_response;
unsigned long t_wait_answer_start;
void loop() {
  if (xQueueReceive(messageQueue, &event, portMAX_DELAY) == pdTRUE) {
    // clear late incoming serial data
    while (UartSerial.available() > 0) {
      UartSerial.read();
    }
    while (Serial.available() > 0) {
      Serial.read();
    }

    // print message to serial bus
    UartSerial.println(event.message);
    Serial.println(event.message);
    digitalWrite(LED_BUILTIN, LOW);

    // prepare respond peer data
    memcpy(respondPeerInfo.peer_addr, &event.mac, 6);
    respondPeerInfo.channel = 0;  
    respondPeerInfo.encrypt = false;
    if(esp_now_add_peer(&respondPeerInfo) != ESP_OK){
      UartSerial.printf("ERROR:Failed to add peer for mac %s\r\n", event.mac);
      Serial.printf("ERROR:Failed to add peer for mac %s\r\n", event.mac);
      return;
    }

    // wait on serial acknowledgement for a maximum of 50 milliseconds
    t_wait_answer_start = millis();
    while(UartSerial.available() == 0 && Serial.available() == 0 && millis() <= t_wait_answer_start + 50) {
      delayMicroseconds(1);
    }

    if (UartSerial.available() > 0) {
      data_response.handled = (UartSerial.read() == ACK);
    }
    else if (Serial.available() > 0) {
      data_response.handled = (Serial.read() == ACK);
    }
    else {
      data_response.handled = false;
    }

    esp_now_send(event.mac, (uint8_t *) &data_response, sizeof(data_response));

    digitalWrite(LED_BUILTIN, HIGH);

    if(esp_now_del_peer(event.mac) != ESP_OK){
      UartSerial.printf("ERROR:Failed to delete peer for mac %s\r\n", event.mac);
      Serial.printf("ERROR:Failed to delete peer for mac %s\r\n", event.mac);
      return;
    }
  }
}
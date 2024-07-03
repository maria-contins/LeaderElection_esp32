#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SPI.h>
#include <MFRC522.h>
#include <esp_now.h>
#include <WiFi.h>

bool leader = false;
bool knowsleader = false;
int id = 0;
unsigned long lastMsgTime = 0;

//MAC
uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint8_t leaderAddress[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// Create peer interface
esp_now_peer_info_t peerInfo;

typedef struct message {
  int type;
  int id;
} message;

message myData;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup initialized.");

  WiFi.mode(WIFI_STA);
  WiFi.channel(1);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized.");

  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  leaderElection();
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  message incomingMsg;
  memcpy(&incomingMsg, incomingData, sizeof(incomingMsg));

  Serial.print("Received message of type: ");
  Serial.println(incomingMsg.type);

  if (incomingMsg.type == 3) {
    memcpy(leaderAddress, mac_addr, 6);
    char macStr[18];
    Serial.print("Packet received from: ");
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);
    // Register peer
    memcpy(peerInfo.peer_addr, leaderAddress, 6);
    peerInfo.channel = 1;  
    peerInfo.encrypt = false;
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
    Serial.print("Leader elected with ID: ");
    Serial.println(incomingMsg.id);
    leader = false; // If we receive an elected message, we are not the leader
    knowsleader = true;
  }
  else if (incomingMsg.type == 2) {
    if (leader) {
      Serial.println("Received check_leader message, sending elected message.");
      message electedMsg;
      electedMsg.type = 3;
      electedMsg.id = id;
      esp_now_send(broadcastAddress, (uint8_t *) &electedMsg, sizeof(electedMsg));
    }
  }
  else if (incomingMsg.type == 1 && leader) {
    Serial.print("Received data message from ID: ");
    Serial.println(incomingMsg.id);
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void leaderElection() {
  Serial.println("Starting leader election.");
  message checkLeaderMsg;
  checkLeaderMsg.type = 2;
  esp_now_send(broadcastAddress, (uint8_t *) &checkLeaderMsg, sizeof(checkLeaderMsg));

  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    // Waiting for elected message
    delay(100);
  }

  if (!knowsleader) { // TODO ??
    Serial.println("No leader found, assuming leadership.");
    leader = true;
    message electedMsg;
    electedMsg.type = 3;
    electedMsg.id = id;
    esp_now_send(broadcastAddress, (uint8_t *) &electedMsg, sizeof(electedMsg));
    knowsleader = true;
  } else {
    Serial.println("Leader found.");
  }
}

void loop() {
  // if (!leader) {
  //   if (millis() - lastMsgTime > 10000) {
  //     Serial.println("Sending random data message.");
  //     message dataMsg;
  //     dataMsg.type = 1;
  //     dataMsg.id = id;
  //     esp_now_send(broadcastAddress, (uint8_t *) &dataMsg, sizeof(dataMsg));
  //     lastMsgTime = millis();
  //   }
  // }
}

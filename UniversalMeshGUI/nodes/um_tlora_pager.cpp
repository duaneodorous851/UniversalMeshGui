#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include "UniversalMesh.h"
#include "ota_update.h"

#ifndef NODE_NAME
  #define NODE_NAME "sensor-node"
#endif
#define HEARTBEAT_INTERVAL  120000
#define TEMP_INTERVAL       120000

UniversalMesh mesh;
uint8_t myMac[6]          = {0};
uint8_t coordinatorMac[6] = {0};
uint8_t meshChannel       = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastTemp      = 0;
volatile bool otaRequested  = false;

static lv_obj_t* lblStatus = nullptr;
static lv_obj_t* lblChannel = nullptr;

static void displayInit() {
  instance.begin();
  beginLvglHelper(instance);
  instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);

  lv_obj_t* screen = lv_screen_active();

  lv_obj_t* lblTitle = lv_label_create(screen);
  lv_label_set_text(lblTitle, "UniversalMesh");
  lv_obj_align(lblTitle, LV_ALIGN_TOP_MID, 0, 8);

  lv_obj_t* lblNode = lv_label_create(screen);
  lv_label_set_text(lblNode, NODE_NAME);
  lv_obj_align_to(lblNode, lblTitle, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);

  lblStatus = lv_label_create(screen);
  lv_label_set_text(lblStatus, "Scanning...");
  lv_obj_center(lblStatus);

  lblChannel = lv_label_create(screen);
  lv_label_set_text(lblChannel, "");
  lv_obj_align_to(lblChannel, lblStatus, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
}

static void displaySetConnected(uint8_t channel, const uint8_t* mac) {
  if (!lblStatus) return;
  lv_label_set_text(lblStatus, "Connected");
  char buf[32];
  snprintf(buf, sizeof(buf), "CH:%d  %02X:%02X:%02X:%02X:%02X:%02X",
           channel, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  lv_label_set_text(lblChannel, buf);
}

static void displaySetScanning() {
  if (!lblStatus) return;
  lv_label_set_text(lblStatus, "Scanning...");
  lv_label_set_text(lblChannel, "");
}

// Send all node info as a single JSON (new 200-byte payload fits everything)
static void sendInfo(uint8_t* destMac, uint8_t appId) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);
  JsonDocument doc;
  doc["n"]    = NODE_NAME;
  doc["mac"]  = macStr;
  doc["up"]   = (unsigned long)(millis() / 1000UL);
  doc["heap"] = ESP.getFreeHeap();
  doc["rssi"] = WiFi.RSSI();
  doc["ch"]   = meshChannel;
  doc["chip"] = ESP.getChipModel();
  doc["rev"]  = (int)ESP.getChipRevision();
  String out;
  serializeJson(doc, out);
  mesh.send(destMac, MESH_TYPE_DATA, appId, out);
}

void onMeshMessage(MeshPacket* packet, uint8_t* senderMac) {
  bool directToMe = (memcmp(packet->destMac, myMac, 6) == 0);
  if (packet->type != MESH_TYPE_DATA || !directToMe) return;

  char msg[201];
  uint8_t len = packet->payloadLen > 200 ? 200 : packet->payloadLen;
  memcpy(msg, packet->payload, len);
  msg[len] = '\0';

  Serial.printf("[RX] From %02X:%02X:%02X:%02X:%02X:%02X | App 0x%02X | %s\n",
          packet->srcMac[0], packet->srcMac[1], packet->srcMac[2],
          packet->srcMac[3], packet->srcMac[4], packet->srcMac[5],
          packet->appId, msg);

  if (len < 4 || strncmp(msg, "cmd:", 4) != 0) return;

  bool fromCoordinator = mesh.isCoordinatorFound() &&
                         (memcmp(packet->srcMac, coordinatorMac, 6) == 0);
  if (!fromCoordinator) {
    Serial.println("[CMD] Ignored — not from coordinator");
    return;
  }

  const char* command = msg + 4;
  char ack[220];
  snprintf(ack, sizeof(ack), "command received:%s", command);
  mesh.send(packet->srcMac, MESH_TYPE_DATA, packet->appId, (const uint8_t*)ack, strlen(ack), 4);

  if (strcmp(command, "info") == 0 || strcmp(command, "info:long") == 0) {
    sendInfo(packet->srcMac, packet->appId);
  } else if (strcmp(command, "reboot") == 0) {
    Serial.println("[CMD] Reboot requested, restarting...");
    delay(100);
    ESP.restart();
  } else if (strcmp(command, "update") == 0) {
    otaRequested = true;
    Serial.println("[CMD] OTA requested");
  }
}

// Discover coordinator, configure mesh, register callback. Returns true on success.
static bool connectToCoordinator() {
  meshChannel = mesh.findCoordinatorChannel(NODE_NAME);
  if (meshChannel == 0) {
    return false;
  }

  mesh.getCoordinatorMac(coordinatorMac);
  mesh.begin(meshChannel);               // re-init on discovered channel
  mesh.setCoordinatorMac(coordinatorMac);
  mesh.onReceive(onMeshMessage);

  esp_wifi_get_mac(WIFI_IF_STA, myMac);

  lastHeartbeat = millis() - HEARTBEAT_INTERVAL;
  lastTemp      = millis() - TEMP_INTERVAL;

  // Announce presence — PING triggers library PONG reply; DATA 0x06 registers name
  mesh.send(coordinatorMac, MESH_TYPE_PING, 0x00, (const uint8_t*)NODE_NAME, strlen(NODE_NAME), 4);
  mesh.send(coordinatorMac, MESH_TYPE_DATA, 0x06, (const uint8_t*)NODE_NAME, strlen(NODE_NAME), 4);
  displaySetConnected(meshChannel, myMac);
  return true;
}

void setup() {
  Serial.begin(115200);
  displayInit();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println();
  Serial.println("========================================");
  Serial.println("       UniversalMesh  -  Sensor Node   ");
  Serial.println("========================================");
  Serial.println("  Scanning for Coordinator...");

  mesh.begin(1);
  mesh.onReceive(onMeshMessage);

  if (connectToCoordinator()) {
    Serial.printf("  Channel     : %d\n", meshChannel);
    Serial.printf("  MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                  myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);
    Serial.println("  Coordinator : Found");
  } else {
    Serial.println("  Coordinator : Not found, retrying in loop...");
  }
  Serial.println("========================================");
}

void loop() {
  if (otaRequested) {
    static bool otaStarted = false;
    if (!otaStarted) {
      otaStarted = true;
      startOtaUpdate();
    }
    delay(20);
    return;
  }

  lv_timer_handler();
  mesh.update();

  if (!mesh.isCoordinatorFound()) {
    displaySetScanning();
    static unsigned long lastRetry = 0;
    if (millis() - lastRetry > 30000) {
      lastRetry = millis();
      Serial.println("[RETRY] Scanning for Coordinator...");
      if (connectToCoordinator()) {
        Serial.printf("[AUTO] Coordinator found on channel %d\n", meshChannel);
      }
    }
    return;
  }

  unsigned long now = millis();

  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = now;
    uint8_t hb = 0x01;
    mesh.send(coordinatorMac, MESH_TYPE_DATA, 0x05, &hb, 1, 4);
    mesh.send(coordinatorMac, MESH_TYPE_DATA, 0x06, (const uint8_t*)NODE_NAME, strlen(NODE_NAME), 4);
    Serial.printf("[TX] Heartbeat | %02X:%02X:%02X:%02X:%02X:%02X\n",
                  myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);
  }

  if (now - lastTemp >= TEMP_INTERVAL) {
    lastTemp = now;
    float tempC = temperatureRead();
    JsonDocument doc;
    doc["name"] = NODE_NAME;
    doc["temp"] = serialized(String(tempC, 1));
    String payload;
    serializeJson(doc, payload);
    if (mesh.sendToCoordinator(0x01, payload)) {
      Serial.printf("[TX] Sensor: %s\n", payload.c_str());
    } else {
      Serial.println("[TX] Send failed");
    }
  }
}

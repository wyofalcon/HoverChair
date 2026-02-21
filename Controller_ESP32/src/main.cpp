#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoOTA.h>
#include <TelnetStream.h>

// ================= HARDWARE CONFIG =================
// Joystick Pins (ADC1 pins only for ESP32 with WiFi/BT on)
#define JOY_X_PIN 34
#define JOY_Y_PIN 35

// Output Serial Pins (Connect to Hoverboard Master)
#define HOVER_TX_PIN 17  
#define HOVER_RX_PIN 16  

// ================= SETTINGS =================
#define JOY_X_CENTER 1805 
#define JOY_Y_CENTER 1803 
#define DEADZONE 150      
#define MAX_SPEED 1000    
const char* ssid = ":]";
const char* password = "qwertasdf";

BluetoothSerial SerialBT;

// ================= ESP-NOW STRUCTURE =================
// Must match the sender (Camera) structure
typedef struct struct_message {
  int16_t x; // Steer
  int16_t y; // Speed
} struct_message;

struct_message incomingData;
volatile int webSpeed = 0;
volatile int webSteer = 0;
volatile unsigned long lastWebRecvTime = 0;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingBytes, int len) {
  memcpy(&incomingData, incomingBytes, sizeof(incomingData));
  webSteer = incomingData.x;
  webSpeed = incomingData.y;
  lastWebRecvTime = millis();
}

// ================= PROTOCOL HELPER =================
// CRC16 Checksum (XMODEM / CCITT)
uint16_t CalcCRC(uint8_t *ptr, int count) {
  uint16_t crc = 0;
  while (--count >= 0) {
    crc = crc ^ (uint16_t)*ptr++ << 8;
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x8000) {
        crc = crc << 1 ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

// Hoverboard Protocol Structure
void sendCommand(Stream &port, int16_t speed, int16_t steer) {
  uint8_t buffer[8];
  uint16_t crc;

  buffer[0] = '/';
  buffer[1] = (uint8_t)((speed >> 8) & 0xFF);
  buffer[2] = (uint8_t)(speed & 0xFF);
  buffer[3] = (uint8_t)((steer >> 8) & 0xFF);
  buffer[4] = (uint8_t)(steer & 0xFF);

  crc = CalcCRC(buffer, 5);
  
  buffer[5] = (uint8_t)((crc >> 8) & 0xFF);
  buffer[6] = (uint8_t)(crc & 0xFF);
  buffer[7] = '\n';

  port.write(buffer, 8);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200); 
  
  // 1. Init WiFi (STA Mode) to sync channels with Router
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long startWifi = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startWifi < 10000) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi Setup Complete (Channel Sync)");

  // 2. Init OTA (Over-The-Air Updates)
  ArduinoOTA.setHostname("HoverChair-Brain");
  ArduinoOTA.begin();
  Serial.println("OTA Ready");

  // 3. Init Telnet Logging
  TelnetStream.begin();
  Serial.println("Telnet Stream Ready");

  // 4. Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    TelnetStream.println("Error initializing ESP-NOW");
  } else {
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Ready!");
  }

  // 5. Init Bluetooth
  SerialBT.begin("HoverChair_Controller"); 
  Serial.println("Bluetooth Started!");

  // 6. Init Hoverboard Serial
  Serial1.begin(19200, SERIAL_8N1, HOVER_RX_PIN, HOVER_TX_PIN);
  
  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);
}

// ================= LOOP =================
void loop() {
  ArduinoOTA.handle(); // Check for OTA updates

  int targetSpeed = 0;
  int targetSteer = 0;
  String source = "";

  bool btActive = false;
  bool webActive = false;

  // --- 1. Check Bluetooth (Priority 1) ---
  static unsigned long lastBtTime = 0;
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    // Simple debug commands
    if (cmd == 'S') { targetSpeed = 0; targetSteer = 0; }
    lastBtTime = millis();
  }
  if (millis() - lastBtTime < 2000) {
    btActive = true;
    source = "BT"; 
    // (Actual BT control logic omitted for brevity, usually phone app sends string)
  }

  // --- 2. Check ESP-NOW / Web (Priority 2) ---
  if (!btActive) {
    if (millis() - lastWebRecvTime < 500) { // Timeout 500ms
      webActive = true;
      targetSpeed = webSpeed;
      targetSteer = webSteer;
      source = "Web";
    }
  }

  // --- 3. Check Physical Joystick (Priority 3) ---
  if (!btActive && !webActive) {
    int joyX = analogRead(JOY_X_PIN);
    int joyY = analogRead(JOY_Y_PIN);

    int deltaX = joyX - JOY_X_CENTER;
    int deltaY = joyY - JOY_Y_CENTER;

    if (abs(deltaX) < DEADZONE) deltaX = 0;
    if (abs(deltaY) < DEADZONE) deltaY = 0;

    if (deltaX > 0) targetSteer = map(deltaX, 0, 4095 - JOY_X_CENTER, 0, MAX_SPEED);
    else if (deltaX < 0) targetSteer = map(deltaX, 0, -JOY_X_CENTER, 0, -MAX_SPEED);

    if (deltaY > 0) targetSpeed = map(deltaY, 0, 4095 - JOY_Y_CENTER, 0, MAX_SPEED);
    else if (deltaY < 0) targetSpeed = map(deltaY, 0, -JOY_Y_CENTER, 0, -MAX_SPEED);

    if (abs(targetSpeed) > 0 || abs(targetSteer) > 0) {
       source = "Joystick";
    } else {
       source = "Idle";
    }
  }

  // --- 4. Send to Hoverboard ---
  sendCommand(Serial1, (int16_t)targetSpeed, (int16_t)targetSteer);

  // Debug output
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 500) {
    // Print to both USB and Telnet
    Serial.printf("Src: %s | Speed: %d | Steer: %d\n", source.c_str(), targetSpeed, targetSteer);
    TelnetStream.printf("Src: %s | Speed: %d | Steer: %d\n", source.c_str(), targetSpeed, targetSteer);
    lastDebug = millis();
  }

  delay(20); 
}

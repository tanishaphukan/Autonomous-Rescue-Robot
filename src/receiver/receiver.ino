/*
  ============================================================
  Autonomous Rescue Robot - RECEIVER (Base Station Side)
  Microcontroller : ESP32 (or Arduino Uno with LoRa shield)
  Communication   : LoRa SX1278
  Output          : Serial Monitor + RC Commands to robot
  ============================================================

  PIN CONNECTIONS:
  ----------------
  LoRa SX1278:
    SCK  -> GPIO 18
    MISO -> GPIO 19
    MOSI -> GPIO 23
    NSS  -> GPIO 5
    RST  -> GPIO 14
    DIO0 -> GPIO 2

  Joystick / Buttons for RC control (optional):
    UP    -> GPIO 34
    DOWN  -> GPIO 35
    LEFT  -> GPIO 32
    RIGHT -> GPIO 33
    STOP  -> GPIO 25
*/

// ---- Libraries ----
#include <SPI.h>
#include <LoRa.h>

// ---- LoRa Pins ----
#define LORA_SCK    18
#define LORA_MISO   19
#define LORA_MOSI   23
#define LORA_NSS     5
#define LORA_RST    14
#define LORA_DIO0    2

// ---- RC Button Pins (optional physical buttons) ----
#define BTN_UP    34
#define BTN_DOWN  35
#define BTN_LEFT  32
#define BTN_RIGHT 33
#define BTN_STOP  25

// ============================================================
void setup() {
  Serial.begin(115200);

  // Button pins
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_STOP,  INPUT_PULLUP);

  // LoRa init
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (true);
  }

  Serial.println("========================================");
  Serial.println("  Rescue Robot - Base Station Ready");
  Serial.println("========================================");
}

// ============================================================
void loop() {
  // --- Receive data from robot ---
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String data = "";
    while (LoRa.available()) data += (char)LoRa.read();
    int rssi = LoRa.packetRssi();

    Serial.println("\n--- Data Received ---");
    parseAndDisplay(data);
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(rssi);
    Serial.println(" dBm");
    Serial.println("---------------------");
  }

  // --- Send RC commands if buttons pressed ---
  if (!digitalRead(BTN_UP))    sendCommand("F");
  if (!digitalRead(BTN_DOWN))  sendCommand("B");
  if (!digitalRead(BTN_LEFT))  sendCommand("L");
  if (!digitalRead(BTN_RIGHT)) sendCommand("R");
  if (!digitalRead(BTN_STOP))  sendCommand("S");

  // --- Also accept keyboard commands via Serial Monitor ---
  if (Serial.available()) {
    char c = Serial.read();
    if      (c == 'w' || c == 'W') sendCommand("F");
    else if (c == 's' || c == 'S') sendCommand("B");
    else if (c == 'a' || c == 'A') sendCommand("L");
    else if (c == 'd' || c == 'D') sendCommand("R");
    else if (c == 'x' || c == 'X') sendCommand("S");
  }

  delay(100);
}

// ============================================================
// Parse incoming data packet and display on Serial Monitor
// Packet format: T:xx.x,P:x,V:x,M:xxxx,LA:xx.xxxxxx,LO:xx.xxxxxx,H:x
void parseAndDisplay(String data) {
  Serial.println("Raw: " + data);

  float temp   = extractFloat(data, "T:");
  int   pir    = extractInt(data, "P:");
  int   vib    = extractInt(data, "V:");
  int   mic    = extractInt(data, "M:");
  float lat    = extractFloat(data, "LA:");
  float lon    = extractFloat(data, "LO:");
  int   human  = extractInt(data, "H:");

  Serial.print("Temperature  : "); Serial.print(temp); Serial.println(" °C");
  Serial.print("PIR Motion   : "); Serial.println(pir  ? "DETECTED"   : "Clear");
  Serial.print("Vibration    : "); Serial.println(vib  ? "DETECTED"   : "Clear");
  Serial.print("Microphone   : "); Serial.print(mic);
  Serial.println(mic > 2500        ? " (HIGH - possible voice)" : " (Normal)");
  Serial.print("GPS Location : Lat="); Serial.print(lat, 6);
  Serial.print("  Lon=");             Serial.println(lon, 6);

  if (human) {
    Serial.println("*** HUMAN DETECTED — ALERT RESCUE TEAM ***");
    Serial.print(">>> Google Maps: https://maps.google.com/?q=");
    Serial.print(lat, 6); Serial.print(","); Serial.println(lon, 6);
  } else {
    Serial.println("Status: No human detected");
  }
}

// ============================================================
// Send RC command to robot via LoRa
void sendCommand(String cmd) {
  LoRa.beginPacket();
  LoRa.print(cmd);
  LoRa.endPacket();
  Serial.println("Command sent: " + cmd);
  delay(200);
}

// ============================================================
// Helper: extract float value after a key in CSV string
float extractFloat(String data, String key) {
  int idx = data.indexOf(key);
  if (idx == -1) return 0.0;
  int start = idx + key.length();
  int end   = data.indexOf(',', start);
  if (end == -1) end = data.length();
  return data.substring(start, end).toFloat();
}

// Helper: extract int value after a key in CSV string
int extractInt(String data, String key) {
  return (int)extractFloat(data, key);
}

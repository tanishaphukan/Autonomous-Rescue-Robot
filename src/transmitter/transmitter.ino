/*
  ============================================================
  Autonomous Rescue Robot - TRANSMITTER (Robot Side)
  Microcontroller : ESP32
  Sensors         : PIR (HC-SR501), Thermal (MLX90640),
                    Microphone (KY-038), Vibration (SW-420),
                    GPS (NEO-6M), IR Obstacle Sensors
  Communication   : LoRa SX1278
  Motor Driver    : L298N (x2)
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

  MLX90640 Thermal Sensor (I2C):
    SDA  -> GPIO 21
    SCL  -> GPIO 22

  GPS NEO-6M (UART2):
    TX   -> GPIO 16
    RX   -> GPIO 17

  PIR Sensor (HC-SR501):
    OUT  -> GPIO 34

  Microphone (KY-038):
    A0   -> GPIO 35

  Vibration Sensor (SW-420):
    OUT  -> GPIO 32

  IR Obstacle Sensors:
    LEFT  -> GPIO 25
    RIGHT -> GPIO 26

  L298N Motor Driver:
    IN1  -> GPIO 27  (Left Motor Forward)
    IN2  -> GPIO 33  (Left Motor Backward)
    IN3  -> GPIO 12  (Right Motor Forward)
    IN4  -> GPIO 13  (Right Motor Backward)
    ENA  -> GPIO 15  (Left Motor Speed - PWM)
    ENB  -> GPIO 4   (Right Motor Speed - PWM)

  Mode Switch:
    SW   -> GPIO 36  (HIGH = RC Mode, LOW = Autonomous Mode)
*/

// ---- Libraries ----
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MLX90640.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

// ---- LoRa Pins ----
#define LORA_SCK    18
#define LORA_MISO   19
#define LORA_MOSI   23
#define LORA_NSS     5
#define LORA_RST    14
#define LORA_DIO0    2

// ---- Sensor Pins ----
#define PIR_PIN       34
#define MIC_PIN       35
#define VIB_PIN       32
#define IR_LEFT       25
#define IR_RIGHT      26
#define MODE_SWITCH   36

// ---- Motor Pins ----
#define IN1  27
#define IN2  33
#define IN3  12
#define IN4  13
#define ENA  15
#define ENB   4

// ---- Thresholds ----
#define TEMP_THRESHOLD   30.0   // °C — human body heat
#define MIC_THRESHOLD    2500   // analog value — distress sound
#define MOTOR_SPEED      180    // 0–255 PWM

// ---- Objects ----
Adafruit_MLX90640 mlx;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);   // UART2 for GPS

float thermalFrame[32 * 24];
bool rcMode = false;

// ============================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // Sensor pins
  pinMode(PIR_PIN, INPUT);
  pinMode(VIB_PIN, INPUT);
  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(MODE_SWITCH, INPUT);

  // Motor pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  stopMotors();

  // LoRa init
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed!");
    while (true);
  }
  Serial.println("LoRa OK");

  // Thermal sensor init
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
  } else {
    mlx.setMode(MLX90640_CHESS);
    mlx.setResolution(MLX90640_ADC_18BIT);
    mlx.setRefreshRate(MLX90640_2_HZ);
    Serial.println("MLX90640 OK");
  }

  Serial.println("Robot Ready.");
}

// ============================================================
void loop() {
  // Update GPS
  while (gpsSerial.available()) gps.encode(gpsSerial.read());

  // Check mode
  rcMode = digitalRead(MODE_SWITCH);

  // Read sensors
  bool pirDetected  = digitalRead(PIR_PIN);
  bool vibDetected  = digitalRead(VIB_PIN);
  int  micValue     = analogRead(MIC_PIN);
  bool micDetected  = (micValue > MIC_THRESHOLD);
  float maxTemp     = getMaxThermalTemp();
  bool tempDetected = (maxTemp > TEMP_THRESHOLD);

  // GPS
  float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
  float lon = gps.location.isValid() ? gps.location.lng() : 0.0;

  // Human detected if 2+ sensors trigger
  int detectionScore = pirDetected + vibDetected + micDetected + tempDetected;
  bool humanDetected = (detectionScore >= 2);

  // Serial output
  Serial.println("-----------------------------");
  Serial.print("Temp: ");     Serial.print(maxTemp);    Serial.println(" C");
  Serial.print("PIR: ");      Serial.println(pirDetected  ? "DETECTED" : "Clear");
  Serial.print("Vibration: ");Serial.println(vibDetected  ? "DETECTED" : "Clear");
  Serial.print("Mic: ");      Serial.print(micValue);   Serial.println(micDetected ? " (HIGH)" : " (Low)");
  Serial.print("GPS: Lat=");  Serial.print(lat, 6);
  Serial.print(" Lon=");      Serial.println(lon, 6);
  Serial.print("Human: ");    Serial.println(humanDetected ? "*** DETECTED ***" : "Not found");

  // Transmit via LoRa
  String packet = "T:" + String(maxTemp, 1) +
                  ",P:" + String(pirDetected) +
                  ",V:" + String(vibDetected) +
                  ",M:" + String(micValue) +
                  ",LA:" + String(lat, 6) +
                  ",LO:" + String(lon, 6) +
                  ",H:" + String(humanDetected ? 1 : 0);

  LoRa.beginPacket();
  LoRa.print(packet);
  LoRa.endPacket();
  Serial.println("Packet sent: " + packet);

  // Movement
  if (rcMode) {
    // RC mode: wait for commands from receiver (handled via LoRa receive)
    handleRCCommand();
  } else {
    // Autonomous mode: IR-based obstacle avoidance
    autonomousNavigate();
  }

  delay(1000);
}

// ============================================================
// Get max temperature from thermal frame
float getMaxThermalTemp() {
  if (mlx.getFrame(thermalFrame) != 0) return 0.0;
  float maxT = thermalFrame[0];
  for (int i = 1; i < 32 * 24; i++) {
    if (thermalFrame[i] > maxT) maxT = thermalFrame[i];
  }
  return maxT;
}

// ============================================================
// Autonomous obstacle avoidance using IR sensors
void autonomousNavigate() {
  bool leftBlocked  = !digitalRead(IR_LEFT);   // LOW = obstacle detected
  bool rightBlocked = !digitalRead(IR_RIGHT);

  if (!leftBlocked && !rightBlocked) {
    moveForward();
  } else if (leftBlocked && !rightBlocked) {
    turnRight();
    delay(400);
  } else if (!leftBlocked && rightBlocked) {
    turnLeft();
    delay(400);
  } else {
    moveBackward();
    delay(500);
    turnRight();
    delay(600);
  }
}

// ============================================================
// RC mode: receive command from base station
void handleRCCommand() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String cmd = "";
    while (LoRa.available()) cmd += (char)LoRa.read();
    cmd.trim();
    Serial.println("RC Command: " + cmd);

    if      (cmd == "F") moveForward();
    else if (cmd == "B") moveBackward();
    else if (cmd == "L") turnLeft();
    else if (cmd == "R") turnRight();
    else if (cmd == "S") stopMotors();
  }
}

// ============================================================
// Motor control functions
void moveForward() {
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void moveBackward() {
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(ENA, MOTOR_SPEED); analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);    analogWrite(ENB, 0);
}

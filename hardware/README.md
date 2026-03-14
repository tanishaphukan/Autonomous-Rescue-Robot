# Hardware Description

## Microcontroller
- **ESP32**- Main controller handling sensor fusion, motor control, and LoRa communication
- **Arduino Uno** - Backup/testing controller

## Sensors

| Sensor | Model | Purpose |
|---|---|---|
| Thermal | MLX90640 | Detects human body heat signatures |
| PIR | HC-SR501 | Detects human motion |
| Microphone | KY-038 | Picks up distress sounds/voices |
| Vibration | SW-420 | Detects movement under debris |
| GPS | NEO-6M | Real-time location tracking (±2m) |
| IR Obstacle | Generic | Autonomous obstacle avoidance |

## Communication
- **LoRa SX1278** - Long-range wireless communication up to 15 km
- **ESP8266** - Wi-Fi module for internet connectivity

## Motor System
- **L298N Motor Driver** - Controls 2 DC motors (dual H-bridge, 2A per channel)
- **DC Motors** - 6V/12V, 100–300 RPM, high torque for rough terrain

## Power Supply
- **Li-ion / Li-Po Battery** - 7.4V or 12V
- **7805 Voltage Regulator** - Provides stable 5V to all components

## Pin Connections (ESP32)

| Component | Pin |
|---|---|
| LoRa SCK | GPIO 18 |
| LoRa MISO | GPIO 19 |
| LoRa MOSI | GPIO 23 |
| LoRa NSS | GPIO 5 |
| LoRa RST | GPIO 14 |
| LoRa DIO0 | GPIO 2 |
| MLX90640 SDA | GPIO 21 |
| MLX90640 SCL | GPIO 22 |
| GPS TX | GPIO 16 |
| GPS RX | GPIO 17 |
| PIR Sensor | GPIO 34 |
| Microphone | GPIO 35 |
| Vibration | GPIO 32 |
| IR Left | GPIO 25 |
| IR Right | GPIO 26 |
| Motor IN1 | GPIO 27 |
| Motor IN2 | GPIO 33 |
| Motor IN3 | GPIO 12 |
| Motor IN4 | GPIO 13 |
| Motor ENA | GPIO 15 |
| Motor ENB | GPIO 4 |

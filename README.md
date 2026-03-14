# 🤖 Autonomous Rescue Robot [Real-Time Human Detection]

> IoT-enabled rescue robot that detects survivors in disaster zones and transmits their GPS location to rescue teams via LoRa.


## What It Does

Navigates disaster-affected environments autonomously, detects human presence using multiple sensors, and wirelessly alerts rescue teams with live location data, reducing risk to human responders.

---

## Key Features

- 🌡️ Multi-sensor detection — thermal, PIR, microphone, vibration
- 📡 LoRa communication — up to 15 km range
- 🗺️ GPS location tracking (NEO-6M, ±2m accuracy)
- 🚗 Hybrid mode — RC control or autonomous IR-based navigation
- ⚙️ 4-wheel drive for rough/debris-filled terrain

---

## Hardware

| Component | Model |
|---|---|
| Microcontroller | ESP32 |
| Thermal Sensor | MLX90640 |
| PIR Sensor | HC-SR501 |
| Microphone | KY-038 |
| Vibration Sensor | SW-420 |
| GPS | NEO-6M |
| Motor Driver | L298N |
| Communication | LoRa + ESP8266 |
| Power | Li-ion/Li-Po 7.4V–12V |

---

## Getting Started

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install libraries: `LoRa`, `TinyGPS++`, `Adafruit MLX90640`, ESP32 board package
3. Upload `transmitter.ino` to the robot ESP32
4. Upload `receiver.ino` to the base station ESP32
5. Open Serial Monitor at 9600 baud

---

## Sample Output

```
Temperature : HIGH (body heat detected)
PIR         : MOTION DETECTED
Microphone  : HIGH (possible voice)
GPS         : Lat=18.6298, Lon=73.8553
Transmitting alert to rescue team...
```

---

## Future Scope

Drone integration · Swarm robotics · Advanced AI detection · Cloud-based remote control

---

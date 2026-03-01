# CAN Bus Intrusion Detection System

Physical-layer authentication for in-vehicle CAN networks using ESP32 and analog signal fingerprinting.

---

## System Architecture

![System Architecture](Project_Files/ARCHITECTURE_IMAGE_.png)

Green = Legitimate nodes | Red = Attacker node | Blue = Surveillance node | Yellow = CAN Bus lines

---

## Overview

Modern vehicles rely on the Controller Area Network (CAN) bus to allow ECUs to communicate
controlling speed, braking, airbags, and more. However, CAN was designed for reliability, not security.
Any node on the bus can send any message with any ID, with zero authentication.

This project implements a real-time Intrusion Detection System that:
- Monitors the CAN bus for unknown or spoofed node IDs
- Uses ADC-based physical-layer fingerprinting to distinguish legitimate ECUs from rogue nodes
- Triggers buzzer and LCD alerts when an intrusion is detected

---

## Project Files

- surviellance_final_demo_code.ino  →  IDS Monitor (main node)
- speed_demo_code.ino               →  Speed ECU (CAN ID: 0x101)
- brake_demo_code.ino               →  Brake ECU (CAN ID: 0x102)
- attacker_code.ino                 →  Attacker Node (CAN ID: 0x666)
- ARCHITECTURE_IMAGE_.png           →  System diagram
- demo_video.mp4                    →  Project demo

---

## Node Summary

| Node             | CAN ID | Role                            |
|------------------|--------|---------------------------------|
| Surveillance IDS |   —    | Monitors bus, detects intrusion |
| Speed ECU        | 0x101  | Sends speed data (payload=60)   |
| Brake ECU        | 0x102  | Sends brake status (payload=1)  |
| Attacker Node    | 0x666  | Injects rogue frames (0xFF)     |

---

## How Detection Works

1. CAN ID Whitelisting
   Only 0x101 and 0x102 are valid. Any other ID is flagged immediately.

2. Physical Layer Analog Fingerprinting
   - ADC pin GPIO 34 samples CAN bus voltage — 50 samples at 12-bit resolution
   - DIFF = MAX sample minus MIN sample is computed for each frame
   - Compared against trained threshold of 350 ADC units
   - Legitimate ECUs produce low stable DIFF
   - Rogue nodes produce high variance DIFF

3. Confirmation Window (prevents false positives)
   - Known ID + DIFF <= 350  →  NORMAL (reset counter, buzzer OFF)
   - Unknown ID              →  +1 abnormalCount
   - DIFF > 350              →  +1 abnormalCount
   - abnormalCount >= 3      →  INTRUSION ALERT (buzzer ON + LCD alert)

---

## Hardware Required

| Component                  | Qty |
|----------------------------|-----|
| ESP32 Dev Board            |  4  |
| MCP2551 CAN Transceiver    |  4  |
| 16x2 I2C LCD (addr 0x27)   |  1  |
| Active Buzzer              |  1  |
| 120 Ohm Resistors          |  2  |
| Push Buttons               |  3  |

---

## Pin Configuration

| GPIO    | Function             | Used By              |
|---------|----------------------|----------------------|
| GPIO 5  | CAN TX               | All nodes            |
| GPIO 4  | CAN RX               | All nodes            |
| GPIO 18 | Button input         | ECU + Attacker nodes |
| GPIO 34 | ADC bus sampling     | Surveillance only    |
| GPIO 27 | Buzzer output        | Surveillance only    |
| SDA/SCL | I2C LCD              | Surveillance only    |

---

## Software Setup

1. Install ESP32 in Arduino IDE
   File → Preferences → Add Board Manager URL:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   Then Tools → Board → Boards Manager → Search esp32 → Install

2. Install Library
   Tools → Manage Libraries → Search LiquidCrystal_I2C → Install
   (driver/twai.h is built into ESP32 core, no install needed)

3. Flash each node
   Open .ino file → Select Board: ESP32 Dev Module → Select COM Port → Upload

---

## Serial Monitor Output (115200 baud)

=== CAN IDS SURVEILLANCE READY ===
[OK] SPEED ECU | STATUS: UPDATED | DIFF=210
[OK] BRAKE ECU | STATUS: UPDATED | DIFF=195
[ALERT] UNKNOWN NODE | CAN ID=0x666 | DIFF=520
[ALERT] UNKNOWN NODE | CAN ID=0x666 | DIFF=515

---

## Test Results

| Node      | CAN ID | Avg DIFF | Threshold | Result    |
|-----------|--------|----------|-----------|-----------|
| Speed ECU | 0x101  |   ~210   |    350    | Normal    |
| Brake ECU | 0x102  |   ~195   |    350    | Normal    |
| Attacker  | 0x666  |   ~520   |    350    | DETECTED  |

---

## Future Scope

- Machine learning based anomaly detection
- CAN FD support for next-gen ECUs
- Cryptographic message authentication (AES / CMAC)
- OBD-II port deployment for real vehicles
- Cloud dashboard for fleet monitoring

---

## Author

pardhu71 — https://github.com/pardhu71

Built as part of an embedded systems security project exploring physical-layer trust in automotive networks.

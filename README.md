# SmartBlinds-ESP32

An automated, IoT-connected blind opener and closer built with an **ESP32**, **A4988 stepper driver**, and **Alexa integration**.

This project transforms standard manual blinds into a smart home asset, allowing for control via a physical hardware button, a mobile app, or Alexa voice commands.

---

## Features

* **Voice Control:** Fully integrated with Amazon Alexa via the SinricPro platform.
* **Mobile Dashboard:** Remote toggle capability from any smartphone via the SinricPro app.
* **Smooth Motion Logic:** Custom C++ implementation featuring linear acceleration and deceleration ramps to reduce mechanical stress on the blind's header rail.
* **Manual Override:** A physical hardware button allows for instant local control without needing a phone or voice.
* **Cloud Sync:** Real-time state synchronization; if you press the physical button, the Alexa app updates the blind status automatically.

---

##  Hardware Needed

* **Microcontroller:** ESP32 (NodeMCU Dev Kit).
* **Motor Driver:** A4988 Stepper Motor Driver.
* **Actuator:** NEMA 17 Stepper Motor.
* **Power Supply:** Dual-rail setup (12V for motor VMOT, 5V via Micro-USB for ESP32 VIN).
* **Stability Components:** 470µF electrolytic decoupling capacitor on the VIN rail to handle Wi-Fi current spikes (~250mA) and prevent brownout resets.



---

## Firmware Overview

The firmware is written in C++ using the Arduino framework, utilizing an **event-driven architecture** rather than simple polling for cloud commands.

### Key Technical Concepts:
* **Asynchronous Callbacks:** Uses `onPowerState` hooks to trigger motor movement immediately upon receiving a cloud packet.
* **Watchdog Maintenance:** The `openCloseRotate2()` function explicitly calls `SinricPro.handle()` during the step loop to prevent the WebSocket connection from timing out during long rotations.
* **Power Management:** Utilizes the A4988 `ENABLE` pin to depower the motor coils when stationary, preventing overheating and saving energy.

### Pin Configuration
| Component | ESP32 GPIO |
| :--- | :--- |
| **Step** | 18 |
| **Direction** | 19 |
| **Enable** | 21 |
| **Button** | 23 |

---

## Installation & Setup

1.  **Library Installation:** Open the Arduino Library Manager and install `SinricPro`, `ArduinoJson`, and `WebSockets`.
2.  **Cloud Configuration:** * Create a free account at [Sinric.pro](https://sinric.pro).
    * Add a "Switch" or "Blinds" device to generate your `APP_KEY`, `APP_SECRET`, and `DEVICE_ID`.
3.  **Alexa Integration:** * Enable the **Sinric Pro** skill in the Alexa app.
    * Link your account and "Discover Devices".
4.  **Deployment:** Update the Wi-Fi and SinricPro credentials in `BlindControlV2.ino` and upload to your ESP32.

---

## Some Troubles I had
During development, I encountered the **ESP32 Brownout Detector** trigger probably due to excessive current draw on boot up. The following hardware mitigations are recommended:
* **Direct VIN Power:** Avoid powering high-draw peripherals from the ESP32's 3.3V regulator; use the `VIN` pin to tap directly into the 5V USB source.
* **Decoupling:** Place a 100µF–470µF capacitor as close to the ESP32 power pins as possible.
* **Common Ground:** Ensure the 12V motor ground and 5V logic ground are tied together to maintain a consistent reference for the `STEP` and `DIR` signals.

---

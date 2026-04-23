![Banner](./Extras/banner.png)

<div align="center">

# TNY-360 Quadruped Robot

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)](https://espressif.com)
[![Framework](https://img.shields.io/badge/Framework-ESP--IDF-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

**A compact, open-source robot dog designed to *Understand*, *Interact*, and *Learn*.**

[🌐 Website](https://tny-robotics.com/tny-360) • [📸 Instagram](https://instagram.com/furwaz_) • [💬 Discord](https://discord.gg/XGABkx5A4y) • [📖 Documentation](https://tny-robotics.com/docs/tny-360/)

</div>

---

## 🚀 Start Building

Everything you need to build the TNY-360 is completely free and open-source.
Follow the links below to find the technical files and documentation you need to get started!

| Resource | Description | Location |
| :--- | :--- | :--- |
| **💻 Firmware** | ESP-IDF source code and drivers. | [`/Firmware`](./Firmware) |
| **📐 CAD & 3D Files** | FreeCAD source files and STLs. | [`/CAD`](./CAD) |
| **📘 Servo Mod Guide** | Instructions to add position feedback to MG996R. | [Documentation](https://tny-robotics.com/docs/tny-360/anatomy/electronics/mg996r-mod) |
| **📖 Assembly Guide** | Step-by-step manual for the full build. | [Documentation](https://tny-robotics.com/docs/tny-360/practical-guide/) |
| **⚡ PCB Design** | All PCBs Gerber, BOM, and Pick'n'Place files. | [`/Electronics/PCBs`](./Electronics/PCBs) |
---

## ✨ Features

* **🦾 High-Performance Motion:** 12 DOF (Degrees of Freedom) using modified servos with position feedback.
* **🧠 AI-Ready Brain:** Powered by the **ESP32-S3 N16R8** for edge computing and Wi-Fi/BLE connectivity.
* **👀 Computer Vision:** Integrated camera (OV2640) for object tracking.
* **🗣️ Interaction:** Features an OLED face, microphone, and speaker for full HRI (Human-Robot Interaction).
* **🛠️ Fully Modifiable:** 100% 3D-printable chassis designed in **FreeCAD** (source files included).

## ⚙️ Hardware Specs

### 🧠 Core & Power
* **MCU:** ESP32-S3 N16R8 Module
* **Power:** 3S LiPo Battery (6x Samsung INR18650-25R)
* **Monitoring:** INA219 (Voltage/Current sensor)

### 🦵 Actuators
* **Legs:** 12x MG996R Servos *(Must be modified for analog feedback)*
* **Head:** 2x SG-90 Micro Servos (Ears)
* **Driver:** PCA9685 (16-Channel PWM)

### 📡 Sensors & I/O
* **Vision:** OV2640 Camera Module
* **Distance:** VL53L0X Time-of-Flight (Lidar)
* **Orientation:** MPU6050 6-axis IMU
* **Display:** SH1106 OLED (128x64)
* **Audio:** I2S MEMS Microphone + Speaker

---

## 📂 Repository Structure

* `BOM/` — **Bill of Materials.** Detailed lists of all components, PCBs, screws, and cables, with links.
* `CAD/` — **Hardware Source.** FreeCAD project files and ready-to-print STLs.
* `Electronics/PCBs/` — **PCB Designs.** Gerber files, BOMs, and Pick'n'Place for all PCBs.
* `Firmware/` — **Codebase.** PlatformIO project (C++/ESP-IDF).
* `Firmware/components/` — **Drivers.** Custom libraries for sensors/actuators.

## 🤝 Contributing

We love contributions!
* **Code:** Please follow the ESP-IDF coding style.
* **Hardware:** Submit PRs with updated FreeCAD files or improvements.
* Found a bug? [Open an Issue](https://github.com/TNY-Robotics/TNY-360/issues).

## 📄 License & Authors

**TNY-360** is maintained by the [TNY Robotics Team](https://tny-robotics.com).

Licensed under **CC BY-NC-SA 4.0**.<br>
*You are free to share and adapt this material for non-commercial purposes, as long as you provide attribution and share alike.*

[![CC BY-NC-SA 4.0](https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)

Need help? Contact us [by mail](mailto:contact@tny-robotics.com) or join our [Discord](https://discord.gg/XGABkx5A4y).

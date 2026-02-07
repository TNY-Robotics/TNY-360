![Banner](./extras/banner.png)

<div align="center">

# TNY-360 Quadruped Robot

[![License: CC BY-NC-SA 4.0](https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-sa/4.0/)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)](https://espressif.com)
[![Framework](https://img.shields.io/badge/Framework-ESP--IDF-orange)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

**A compact, open-source robot dog designed to *Understand*, *Interact*, and *Learn*.**

[🌐 Website](https://tny-robotics.com/tny-360) • [📸 Instagram](https://instagram.com/furwaz_) • [💬 Discord](https://discord.gg/XGABkx5A4y)

</div>

---

## 🚀 Start Building

Want to build your own? We provide resources ranging from free open-source files to complete "Maker Kits" to save you time.

| Resource | Description | Access |
| :--- | :--- | :--- |
| **📁 Source Code & 3D** | Full access to Firmware, FreeCAD files & BOM. | [**Free on GitHub**](#) |
| **📘 Servo Mod Guide** | PCB files, Bill Of Materials and instructions to add position feedback to MG996R servos. | [**Get for 3€**](https://furwaz.gumroad.com/l/tny-servo-v1) |
| **📖 Assembly guides** | Step-by-step manual to assemble and get started with your TNY-360, worry-free. | [**Get for 5€**](https://furwaz.gumroad.com/l/tny-360-guides) |
| **⚡ PCB Pack** | Custom main board, motor controller and other PCB files + BOM (Ready for JLCPCB & LCSC). | [**Get for 9€**](https://furwaz.gumroad.com/l/tny-360-pcb-pack) |
| **🧰 The Maker Kit** | **(Best Value)** All PCB files + Servo Guide + Full Assembly Manual. | [**Get for 14€**](https://furwaz.gumroad.com/l/tny-360-maker-kit) |

> 💡 **Support the project:** Buying these guides directly supports this project and the development of future TNY robots!

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

## 💻 Firmware Quick Start

The firmware is built on **PlatformIO** using the **ESP-IDF** framework.

### Prerequisites
1.  **VS Code** installed.
2.  **PlatformIO** extension installed in VS Code.

### Installation
1.  **Clone** this repository:
    ```bash
    git clone --recursive https://github.com/TNY-Robotics/TNY-360.git
    ```
2.  **Open** the `Firmware/` folder in VS Code.
3.  **Connect** your ESP32-S3 via USB.
4.  **Upload:** Click the generic "Arrow" icon (➡️) in the PlatformIO bottom bar to Build & Flash.

> **Note:** If you are missing drivers, run `git submodule update --init --recursive` to pull the TNY-Robotics custom components.

---

## 📂 Repository Structure

* `CAD/` — **Hardware Source.** FreeCAD project files and ready-to-print STLs.
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
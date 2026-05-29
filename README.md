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

## 🚀 How to Build Your Own TNY-360

Everything you need to build this robot is 100% free and open-source. Instead of getting lost in the folders, we highly recommend following our official, step-by-step wiki:

<div align="center">

> ### **[👉 START HERE: The Official Step-by-Step Guide 👈](https://tny-robotics.com/docs/tny-360/practical-guide/sourcing)**
> *From ordering the first screw to the assembing the last part, this guide covers it all.*

</div>

### 🗂️ Quick Links (For Advanced Makers)
If you know what you are doing and just want the raw files, here is your toolkit:

* 🛒 **[Bill of Materials (BOM)](https://tny-robotics.com/docs/tny-360/practical-guide/1.sourcing/)** - What to buy, tools needed, and where to find them.
* 📐 **[CAD & 3D Files](./CAD)** - Grab the `.STL` and `.3MF` files for your 3D printer.
* ⚡ **[PCB Designs (Gerbers)](./Electronics/PCBs)** - Files to order the custom PCBs used in the robot.
* 🧠 **[ESP32 Firmware](./Firmware)** - The ESP-IDF C++ firmware & drivers powering the robot.
* 🛠️ **[Servo Modding Tutorial](https://tny-robotics.com/docs/tny-360/anatomy/electronics/mg996r-mod)** - Learn the Op-Amp trick to get position-feedback on MG996R servos.

## ✨ Features

* 🧬 **Dual-Core Architecture:**<br> A segregated system using the ESP32-S3. Core 0 ("Reflex") runs a strict 200Hz kinematics/control loop, while Core 1 ("Brain") handles WebSockets, UI, and orchestration.
* 🔄 **Closed-Loop at 200Hz:**<br> We ditched blind PWM. The TNY-360 uses digital MG996R servos modified with a custom **Op-Amp buffer PCB** for noise-free, high-speed position feedback.
* ⚡ **Zero Cable Spaghetti:**<br> A fully modular hardware ecosystem. Dedicated PCBs (Control, Sensor, Brain, Power, etc.) communicate cleanly via standardized JST-PH cables.
* 🛠️ **Smart Auto-Calibration:**<br> Features mechanical endstops for assembly validation and runtime algorithms that compensate for backlash and actuator latency.
* 🧩 **STEM & Expansion Ready:**<br> Program the robot using [TNY-Coder](https://github.com/TNY-Robotics/TNY-Coder) (our block-based web app) or build custom hardware modules using the dorsal I2C/Power expansion port.
* 🔋 **Industrial Power:**<br> A custom 3S 18650 battery pack capable of 15A continuous discharge, featuring an integrated BMS and Fuse, for 1h+ of runtime.

## 🧠 The Engineering: Beyond a Budget Robot

Most budget quadrupeds suffer from analog noise and jitter when hacking servos for feedback. Simply soldering a wire to a servo's internal potentiometer creates an antenna for EMI and draws current that destabilizes the servo's internal control loop.

**The TNY-360 Solution:** We designed a custom micro-PCB that fits inside the servo, featuring an **Operational Amplifier (Op-Amp) in a voltage follower (buffer) configuration**. 
This provides near-infinite input impedance (zero interference with the motor) and low output impedance to send a clean analog signal over the robot's wiring harness. Coupled with a software low-pass filter, the ESP32 can cleanly poll all 12 legs at 200Hz.

👉 **[Read the full technical deep-dive and the step-by-step Servo Modding Guide on our documentation site.](https://tny-robotics.com/docs/tny-360/anatomy/electronics/mg996r-mod)**

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

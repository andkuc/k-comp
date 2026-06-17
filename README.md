# 🖥️ K-Comp Single Board Computer

Welcome to the official repository for the **K-Comp**, an educational Single Board Computer (SBC) engineered to teach embedded systems, C/C++ architecture, and hardware reality. 

Built around the industry-standard **ATmega328P** microcontroller, the K-Comp is, at its core compatible to the Arduino Uno. However, it reimagines the early learning experience by integrating core peripherals directly onto the PCB, eliminating the electrical noise and frustration of complex breadboard wiring. 

It is designed to give students a stable foundation to stand on, while demanding they take true ownership of their code, memory, and logic.

---

## ⚖️ Architectural Distinction: K-Comp vs. Traditional Approaches

When evaluating educational hardware, systems generally fall into one of two extremes. The K-Comp was designed specifically to bridge the gap between them, establishing a third path built on structural integrity and scalable complexity.

* **High-Abstraction Frameworks (The "Black Box"):** Many commercial STEM kits prioritize immediate success by hiding the microcontroller's physical reality. They abstract away memory limits, clock speeds, and hardware polling behind heavy, proprietary APIs. While beginners can build projects quickly, they learn the API rather than the underlying computer science. 
* **Raw Microcontrollers & Breadboards:** The traditional alternative is handing a student a bare board and jumper wires. While this offers ultimate freedom, it lacks structural boundaries. Students often spend 80% of their time debugging loose wires, floating grounds, and electrical noise, leaving little room to focus on logical software design.
* **The K-Comp Synthesis (Opt-In Scaffolding):** The K-Comp integrates core peripherals onto a single PCB to eliminate wiring noise, providing a stable, mathematically pure perimeter. It provides highly engineered scaffolding to help students build complex systems without doing the thinking for them. Crucially, this scaffolding is strictly "opt-in." The board does not force a proprietary ecosystem; students can discard the libraries entirely and write standard C or bare-metal Assembler when they are ready.

---

## 🧭 The Pedagogical Philosophy: The Sliding Scale of Abstraction

Because the K-Comp refuses vendor lock-in, it operates on a **Sliding Scale of Abstraction**. It meets the student exactly where they are and allows them to peel back the layers at their own pace:

* **Level 1 (Visual Logic):** Support for **mBlock 5**, allowing absolute beginners to grasp logical flows, loops, and hardware control via visual blocks.
* **Level 2 (Scaffolded C):** Programming in the **Arduino IDE** or **PlatformIO**/**Visual Studio Code** using the custom K-Comp library ecosystem. These libraries handle complex tasks while forcing the student to explicitly manage static memory allocation.
* **Level 3 (Arduino/AVR C/C++):** Because it is an ATmega328P, students can bypass the K-Comp tools entirely and use standard Arduino/AVR /CC++ and third-party open-source libraries.
* **Level 4 (Bare Metal):** The hardware hides nothing. Advanced students can write direct port manipulation code, utilize hardware interrupts, or write pure AVR Assembler. 

---

## ⚡ The Hardware Reality

The K-Comp exposes students to the strict constraints of real silicon—a 16 MHz clock and just 2 KB of SRAM—forcing them to learn optimization and structural integrity early.

### 1. The Core & Onboard Peripherals
To ensure students can immediately focus on writing logical architecture rather than debugging loose wires, the K-Comp features a suite of pre-routed, noise-free onboard hardware:
* **Visuals:** 0.96" OLED Display (128x64 I²C matrix) and indicator LEDs.
* **Inputs:** 2-Axis Analog Joystick (KY-023), tactile `START` / `SELECT` buttons, and an analog potentiometer.
* **Audio:** Passive buzzer for frequency-based tone generation.

### 2. The Expansion Ecosystem (The Assortment Box)
The onboard peripherals are just the starting point. The K-Comp is designed to interface seamlessly with standard external components. Featuring exposed I²C, UART, Analog, PWM, and Digital headers, the system effortlessly scales to support a massive library of industry-standard modules.

---

## 📚 The Software Ecosystem (Opt-In Scaffolding)

This repository includes a custom framework designed specifically for the K-Comp. Unlike standard bloated libraries, these tools are built with memory efficiency and architectural transparency, allowing students to interact with the hardware seamlessly while maintaining total control over the system's limits.

### Standard Libraries (Onboard Peripherals)
These libraries provide immediate, clean access to the K-Comp's pre-routed hardware:
* **Display Engine:** `kcomp_oled.h` and `kcomp_oled_paged.h`
* **Interactive Inputs:** `kcomp_joystick.h` 
* **Asynchronous Events:** `kcomp_button_start.h` & `kcomp_button_select.h` 
* **Audio/Visual Feedback:** `kcomp_buzzer.h` & `kcomp_led.h`
* **Analog Measurement:** `kcomp_potentiometer.h` 

### Extension Libraries (The Assortment Box)
As students outgrow the onboard peripherals, the K-Comp ecosystem scales via dedicated extension headers. The framework supports a wide array of external components, categorized by their architectural function:

* **Environmental & Atmospheric Sensors:**
  * *Examples:* BMP280 (Temp/Pressure), AHT10 (Temp/Humidity), DS18B20 (Waterproof Temp), SPS-30 (Particulate Matter), MQ-Series Gas Sensors (MQ-2 Smoke, MQ-3 Ethanol, MQ-7 CO, MQ-135 Air Quality).
* **Biometric & Health Monitoring:**
  * *Examples:* MAX30102 (Heart Rate/SPO2), AD8232 (ECG Module), KY-039 (Heartbeat Detector).
* **Spatial, Proximity & Human Presence:**
  * *Examples:* HC-SR04 (Ultrasonic), TOF0400C VL53L1X (Laser Distance), LD2410C / LD2450C (Human Presence Radar), PAJ7620U2 (Hand Gesture Recognition).
* **Optical, Light & Color:**
  * *Examples:* AS7343 (380-1000nm Spectrometer), AS7331 (UV-A/B/C Sensor), TCS34725 (RGB Color Sensor), GY-302 (Light Intensity).
* **Motion, Force & Actuation:**
  * *Examples:* SG90 Servos (180°/360°), DC Motors with TB6612FNG Dual Drivers, HX711 (50kg Load Cells for Weight), BMI-160 (Gyroscope/Accelerometer), DRV2605L (Haptic Vibration Driver), 9000rpm Vibration Motors.
* **Wireless & Communication:**
  * *Examples:* ESP-8266 (WiFi), HC-05 (Bluetooth), GY-NEO6M (GPS), HC-12 (433MHz Transceiver), IR RX/TX (940nm).
* **Advanced Interface & Identification:**
  * *Examples:* AS608 TZT (Fingerprint Reader), PN532 (NFC Controller), I²C QWERTY Keyboards, CH9329 (USB HID Emulation), 1.28'' RGB TFT Displays, NFC Tags (215).

**The Rule of Choice:** All libraries are provided as robust educational scaffolding. However, students always maintain absolute authority to ignore them, use standard Arduino open-source libraries, or write their own hardware drivers from scratch using the raw datasheets.

---

## 🗂️ Repository Roadmap

This repository contains everything required to manufacture, setup, and code the K-Comp.

* 📂 **`/board-definition`**: The Arduino IDE JSON definitions and hardware files to install the K-Comp Board Manager profile.
* 📂 **`/libraries`**: The complete suite of custom K-Comp C++ libraries (Display, Grid Engine, Inputs, Sensors) with examples. The libraries are distributed with the board definition as well and are listed here for browsing. 
* 📂 **`/docs`**: In-depth API documentation, architectural concept explainers, and the physical Assembly Guide.
* 📂 **`/hardware`**: Gerber files, PCB schematics, and the Bill of Materials (BOM) for manufacturing your own boards.
* 📂 **`/examples`**: A curated folder of sample projects, ranging from basic hardware diagnostics to fully realized interactive systems. 

---
*For setup instructions, IDE installation, or assembly steps, please refer to the specific manuals located in the `/docs` folder.*
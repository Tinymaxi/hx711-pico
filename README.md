# hx711-pico

C++ driver for the [hx711 24-bit ADC](https://www.mouser.com/datasheet/2/813/hx711_english-1022875.pdf) using the Raspberry Pi Pico (RP2350) and the Pico SDK.  
The hx711 is commonly used with load cells to build digital scales.  
This repository provides a modern C++ class for the Pico, plus calibration examples.

---

## ✨ Features
- PIO-based hx711 interface for reliable sampling
- Read raw 24-bit signed values
- Tare (zero) function
- Scale calibration (counts ↔ grams)
- Power down / power up control
- Example program for calibration and weight reading
- CMake build system (works with Pico SDK 2.x)

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).

- Parts of this code are derived from [Rob Tillaart’s hx711 library](https://github.com/RobTillaart/hx711),  
  Copyright (c) 2019-2025 Rob Tillaart, MIT License.
- Parts of this code are derived from [Daniel Robertons hx711 PIO](https://github.com/endail/hx711),  
  Copyright (c) 2020 Daniel Robertson, MIT License.
- Additions and Pico-specific code  
  Copyright (c) 2025 Max Penfold, MIT License.


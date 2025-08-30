![VS Code](https://img.shields.io/badge/VS%20Code-Raspberry%20Pi%20Extension-blue?logo=visualstudiocode)
![Release](https://img.shields.io/badge/release-v0.1.0-blue.svg)
![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)

# hx711-pico

C++ driver for the [hx711 24-bit ADC](https://www.mouser.com/datasheet/2/813/hx711_english-1022875.pdf) using the Raspberry Pi Pico (RP2350) and the Pico SDK.  
The hx711 is commonly used with load cells to build digital scales.  
This repository provides a modern C++ class for the Pico, plus calibration examples.

---

## ✨ Features
- Supports the HX711 24-bit ADC for load cells
- Fixed configuration: Gain = 128, Output Rate = 10 samples per second (SPS)
- Simple calibration routine with tare and scale factor
- Example applications included:
  - Calibration
  - Raw sensor value reading
  - Saving the calibration to flash memory
    enablig weighin without new calibratin after powering back on.
- Compatible with Raspberry Pi RP2350 and Pico SDK (CMake build system)
- Uses RP2350’s Programmable I/O (PIO) for efficient sampling
- **Parallel reads:** Each RP2350 PIO block can run four state machines, 
  allowing up to four HX711 load cells to be read in parallel

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).

- Parts of this code are derived from [Rob Tillaart’s hx711 library](https://github.com/RobTillaart/hx711),  
  Copyright (c) 2019-2025 Rob Tillaart, MIT License.
- Parts of this code are derived from [Daniel Robertons hx711 PIO](https://github.com/endail/hx711),  
  Copyright (c) 2020 Daniel Robertson, MIT License.
- Additions and Pico-specific code  
  Copyright (c) 2025 Max Penfold, MIT License.

## Info
cpg is counts per gram.
  grams = (raw - offset) / cpg


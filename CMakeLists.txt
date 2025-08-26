# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Fallback for SDK path
if(NOT DEFINED PICO_SDK_PATH)
  set(PICO_SDK_PATH "${USERHOME}/.pico-sdk/sdk/${sdkVersion}")
endif()

include(pico_sdk_import.cmake)
project(NewHX711 C CXX ASM)  # ← project name (consistent caps)
pico_sdk_init()

# ── HX711 driver library ───────────────────────────────────────────
add_library(hx711 STATIC
  HX711.cpp
)

# Generate PIO header for the LIB (not the exe)
pico_generate_pio_header(hx711 ${CMAKE_CURRENT_LIST_DIR}/hx711_reader.pio)

# Public include so examples can #include "HX711.hpp"
target_include_directories(hx711 PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

# Link SDK libs to hx711 (PUBLIC so dependents inherit usage)
target_link_libraries(hx711 PUBLIC
  pico_stdlib
  hardware_pio
  hardware_clocks
  hardware_gpio
  hardware_timer
)

# (optional) enable float printf for anything that links hx711
target_compile_definitions(hx711 PUBLIC
  PICO_PRINTF_SUPPORT_FLOAT=1
)

# ── Executable: NewHX711 (examples/calibration/main.cpp) ───────────
add_executable(NewHX711
  examples/calibration/main.cpp
)

# USB CDC for printf (UART off)
pico_enable_stdio_uart(NewHX711 0)
pico_enable_stdio_usb(NewHX711 1)

target_link_libraries(NewHX711
  hx711        # our driver lib
  pico_stdlib  # stdio, etc.
)

target_include_directories(NewHX711 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_set_program_name(NewHX711 "NewHX711")
pico_set_program_version(NewHX711 "0.1")

pico_add_extra_outputs(NewHX711)
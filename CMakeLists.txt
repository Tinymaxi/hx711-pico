# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
project(NewHx711 C CXX ASM)
pico_sdk_init()

add_executable(NewHx711
  main.cpp
  hx711.cpp
  hx711.c
  scale.c
)

# Generate header from your existing PIO program
pico_generate_pio_header(NewHx711 ${CMAKE_CURRENT_LIST_DIR}/hx711_reader.pio)

target_sources(NewHx711 PRIVATE
    main.cpp
    hx711.cpp
    scale.c
    hx711.c 
)

pico_set_program_name(NewHx711 "NewHx711")
pico_set_program_version(NewHx711 "0.1")

# USB CDC for printf (UART off)
pico_enable_stdio_uart(NewHx711 0)
pico_enable_stdio_usb(NewHx711 1)

target_link_libraries(NewHx711
  pico_stdlib
  hardware_pio
  hardware_clocks
  hardware_gpio
)

target_include_directories(NewHx711 PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_enable_stdio_uart(NewHx711 0)
pico_enable_stdio_usb(NewHx711 1)

pico_add_extra_outputs(NewHx711)
        


// hx711.cpp - hx711 driver for Raspberry Pi Pico
// Copyright (c) 2025 Max Penfold
// MIT License
//
// Portions derived from hx711 Arduino library
// Copyright (c) 2019-2025 Rob Tillaart
// MIT License

#include "hx711.hpp"
#include "pico/stdlib.h"
#include <iostream>

int main()
{
    stdio_init_all();

    // --- Wait until the Serial Monitor is open ---
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    std::cout << "Serial connected. Ready.\r\r\n";

    hx711 myScale(16, 17);

    // Step 1: print instructions
    std::cout << "\r\n\r\nCALIBRATION\r\n"
              << "===========\r\n"
              << "Remove all weight from the load cell\r\n"
              << "place your cursor in the message box of the serial monitor\r\n"
              << "and press Enter.\r\n"
              << std::flush;

    // Step 2: wait until user hits Enter
    std::string line;
    std::getline(std::cin, line); // this blocks until Enter is pressed

    // Step 3: react
    std::cout << "Determining zero weight offset.\r\n"
              << std::flush;

    myScale.tare(20);
    int32_t offset = myScale.get_offset();

    std::cout << "OFFSET: " << offset << "\r\n\r\n"
              << "Place a weight on the load cell, \n"
              << "ideally 80 to 90 % of the load cells capacity, then \n"
              << "type in the weight in (whole) grams and press enter.\n";

    int weight;
    if (std::cin >> weight)
    {
        std::cout << "WEIGHT: " << weight << "\r\n";
        ;
    }

    // std::cout << "WEIGHT: " << weight << "\r\n";
    myScale.calibrate_scale(weight, 20);
    float scale = myScale.get_scale();

    std::cout << "\nUse myScale.set_offset("
              << offset
              << "); and myScale.set_scale("
              << scale
              << ");\n";

    std::cout << "in the setup of your project.\n";

    myScale.set_offset(offset);
    myScale.set_scale(scale);

    for (size_t i = 0; i < 20; i++)
    {
        int32_t scaleOffset = myScale.get_offset();
        float scaleFactor = myScale.get_scale();
        int32_t v = myScale.read_raw_hx711();
        int32_t readWithOffset = v - scaleOffset;
        float g = readWithOffset / scaleFactor;

        printf("Scale read_raw_hx711: %d with Offset: %d weight = (raw_hx711 - offset) / scale factor : %.2f\r\n ", v, readWithOffset, g);
        sleep_ms(100);
    }

    for (size_t i = 0; i < 20; i++)
    {
        float result = myScale.read_weight();
        printf("Weight : %.2f\n", result);
        sleep_ms(100);
    }
}
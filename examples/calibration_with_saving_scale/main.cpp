// hx711.cpp - hx711 driver for Raspberry Pi Pico
// Copyright (c) 2025 Max Penfold
// MIT License
//
// Portions derived from hx711 Arduino library
// Copyright (c) 2019-2025 Rob Tillaart
// MIT License

// In the CMakeLists.txt under add_executable(..), comment out the main.cpp files you don't need.

#include "hx711.hpp"
#include "config_store.hpp"
#include "pico/stdlib.h"
#include <iostream>
#include <limits>

int main()
{
    stdio_init_all();

    // --- Wait until the Serial Monitor is open ---
    while (!stdio_usb_connected())
    {
        sleep_ms(500);
    }

    std::cout << "Serial connected. Ready.\r\r\n";

    hx711 myScale(16, 17);

    ScaleConfig sc;
    if (load_scale_config(sc))
    {
        std::printf("Loaded config\r\n");
        apply_scale_config(myScale, sc.entries[0]);
        std::printf("Scale now: %.6f, Offset: %ld\r\n",
                    myScale.get_scale(),
                    (long)myScale.get_offset());
    }
    else
    {
        std::printf("No valid config in flash.\n");
    }

    std::cout << "Type 0 for calibration and 1 for skipping it.\r\n"
              << std::flush;

    int32_t ScaleWasSet = 0;
    if (std::cin >> ScaleWasSet)
    {
        std::cout << "INPUT: " << ScaleWasSet << "\r\n";
    }

    // Eat the pending newline before any getline()
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (ScaleWasSet == 0)
    {
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
        }

        myScale.calibrate_scale(weight, 20);

        float scale = myScale.get_scale();

        std::cout << "\nUse myScale.set_offset("
                  << offset
                  << "); and myScale.set_scale("
                  << scale
                  << ");\n";
        std::cout << "in the setup of your project.\n";

        // --- Save AFTER populating sc ---
        sc = {}; // optional clear/reset if your struct benefits from it
        sc.entries[0].offset_counts = myScale.get_offset();
        sc.entries[0].count_per_g = myScale.get_scale();

        if (save_scale_config(sc))
        {
            std::printf("Config saved.\r\n");
        }
        else
        {
            std::printf("ERROR: saving config failed.\r\n");
        }

        // Quick verification prints
        for (size_t i = 0; i < 20; i++)
        {
            const int32_t scaleOffset = sc.entries[0].offset_counts;
            const float scaleFactor = sc.entries[0].count_per_g;

            int32_t raw = myScale.read_raw_hx711();
            int32_t net = raw - scaleOffset;
            float grams = net / scaleFactor;

            // raw, offset, net, grams
            printf("raw=%ld, offset=%ld, net=%ld, grams=%.2f\r\n",
                   (long)raw, (long)scaleOffset, (long)net, grams);
            sleep_ms(100);
        }
    }
    else if (ScaleWasSet > 0)
    {
        sleep_ms(2000);
        float scale = myScale.get_scale();
        float offset = myScale.get_offset();
        printf("The scale is: %.6f, The offset is: %.6f\r\n", (double)scale, offset);

        while (true)
        {
            float grams = myScale.read_weight_trimmed_mavg(); // one sample per 100 ms
            printf("The weight is: %.2f\n", grams);
            sleep_ms(100);
        }
    }
}
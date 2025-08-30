#include "config_store.hpp"
#include "hx711.hpp"
#include "pico/stdlib.h"
#include <cstdio>

int main() {
    stdio_init_all();

        // --- Wait until the Serial Monitor is open ---
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    hx711 scale1(16, 17);
    hx711 scale2(18, 19);
    hx711 scale3(20, 21);

    
    ScaleConfig sc;
    if (load_scale_config(sc)) {
        std::printf("Loaded config\n");
        apply_scale_config(scale1, sc.entries[0]);
        apply_scale_config(scale2, sc.entries[1]);
        apply_scale_config(scale3, sc.entries[2]);
    } else {
        std::printf("No valid config in flash.\n");
    }

    // After calibration:
    sc.entries[0].offset_counts = scale1.get_offset();
    sc.entries[0].g_per_count   = scale1.get_scale();

    sc.entries[1].offset_counts = scale2.get_offset();
    sc.entries[1].g_per_count   = scale2.get_scale();

    sc.entries[2].offset_counts = scale3.get_offset();
    sc.entries[2].g_per_count   = scale3.get_scale();

    save_scale_config(sc);

    while (true) tight_loop_contents();
}
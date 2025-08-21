#include <stdio.h>
#include "pico/stdlib.h"
#include "hx711.hpp"   // your C++ wrapper

int main() {
    stdio_init_all();

    // Use your actual GPIO numbers
    const int CLOCK_PIN = 16;
    const int DATA_PIN  = 17;

    Hx711 hx(CLOCK_PIN, DATA_PIN);
    hx.power_up(hx711_gain_128);

    sleep_ms(1000); // let HX711 settle a bit

    printf("Raw HX711 counts:\n");
    while (true) {
        // get one 24-bit reading directly
        int32_t val = hx.get_value();
        printf("%ld\n", (long)val);
        sleep_ms(100);  // ~10 samples/s at RATE=0, so 100ms is safe
    }
}
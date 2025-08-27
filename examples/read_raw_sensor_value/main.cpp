#include "hx711.hpp"
#include "pico/stdlib.h"
#include <cstdio>

// Each PIO block on the Pico has four state machines.
// This allows up to four HX711 devices to be read in parallel per PIO block.

int main()
{
    stdio_init_all();

    hx711 myScale1(16, 17);
    hx711 myScale2(18, 19);
    hx711 myScale3(20, 21);

    
    while (true)
    {
        int32_t raw1 = myScale1.read_raw_hx711();
        int32_t raw2 = myScale2.read_raw_hx711();
        int32_t raw3 = myScale3.read_raw_hx711();

        printf("Raw sensor value: Scale 1,  %6d ,Scale 2,  %6d ,Scale 3,  %6d ,\n", raw1, raw2, raw3);
    }
}
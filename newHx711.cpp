#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hx711_reader.pio.h" 


int main()
{
    stdio_init_all();

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &hx711_reader_program);
    uint sm = pio_claim_unused_sm(pio, true);

    const uint CLOCK_PIN = 16;
    const uint DATA_PIN = 17;


    hx711_reader_pio_init(pio, sm, offset, DATA_PIN, CLOCK_PIN);
    hx711_reader_program_init(pio, sm, offset, DATA_PIN, CLOCK_PIN);

    while (true)
    {

        // Blocking read from FIFO (if your PIO pushes data)
        if (!pio_sm_is_rx_fifo_empty(pio, sm))
        {
            uint32_t value = pio_sm_get_blocking(pio, sm);
            printf("HX711 value: %ld\n", value);
        }

    }
}
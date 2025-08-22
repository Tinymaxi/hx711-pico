#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hx711_reader.pio.h"

class HX711
{
public:
    HX711(uint clockPin, uint dataPin)
        : clockPin_(clockPin), dataPin_(dataPin)
    {
        // Add the program once on this PIO
        if (!programLoaded){
            programOffset = pio_add_program(pio_, &hx711_reader_program);
            programLoaded = true;
        } 
        sm_ = pio_claim_unused_sm(pio_, true);
        hx711_reader_pio_init(pio_, sm_, programOffset, dataPin_, clockPin_);
        hx711_reader_program_init(pio_, sm_, programOffset, dataPin_, clockPin_);
    }

    int32_t read() {
        uint32_t raw = pio_sm_get_blocking(pio_, sm_);

        // Sign-extend 24 - 32 bits
        if (raw & 0x00800000) {
            raw |= 0xFF000000;
        }

        return static_cast<int32_t>(raw);
    }

private:
    uint clockPin_, dataPin_;
    uint sm_;
    PIO pio_ = pio0;
    static inline bool programLoaded = false;
    static inline uint programOffset = 0;
};

int main()
{
    stdio_init_all();

    HX711 myScale1(16,17);
    // HX711 myScale2(18,19);
    // HX711 myScale3(20,21);

    while (true)
    {
        int32_t v1 = myScale1.read();
        // int32_t v2 = myScale2.read();
        // int32_t v3 = myScale3.read();
        printf("my scale1: %ld\n", v1);
        // printf("my scale2: %ld\n", v2);
        // printf("my scale3: %ld\n", v3);
        // printf("hello");

    }
}


#include <ctype.h>
#include <stdio.h>
// #include <cstdio>
// #include <string>
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
        if (!programLoaded)
        {
            programOffset = pio_add_program(pio_, &hx711_reader_program);
            programLoaded = true;
        }
        sm_ = pio_claim_unused_sm(pio_, true);
        hx711_reader_pio_init(pio_, sm_, programOffset, dataPin_, clockPin_);
        hx711_reader_program_init(pio_, sm_, programOffset, dataPin_, clockPin_);
    }

    ///////////////////////////////////////////////////////////////
    //
    //  READ
    //
    //  From datasheet page 4
    int32_t read()
    {
        uint32_t raw = pio_sm_get_blocking(pio_, sm_);

        // Sign-extend 24 → 32 bits
        if (raw & 0x00800000)
        {
            raw |= 0xFF000000;
        }

        return static_cast<int32_t>(raw);
    }

    float readAverage(uint8_t times)
    {
        if (times < 1)
            times = 1;
        float sum = 0;
        for (uint8_t i = 0; i < times; i++)
        {
            sum += read();
        }
        return sum / times;
    }

    ///////////////////////////////////////////////////////
    //
    //  TARE
    //
    void tare(uint8_t times)
    {
        _offset = readAverage(times);
    }

    float getTare()
    {
        return -_offset * _scale;
    }

    bool tareSet()
    {
        return _offset != 0;
    }

    ///////////////////////////////////////////////////////////////
    //
    //  CALIBRATION  (tare see above)
    //
    bool setScale(float scale)
    {
        if (scale == 0)
            return false;
        _scale = 1.0 / scale;
        return true;
    }

    float getScale()
    {
        return 1.0 / _scale;
    }

    void setOffset(int32_t offset)
    {
        _offset = offset;
    }

    int32_t getOffset()
    {
        return _offset;
    }

    //  assumes tare() has been set.
    void calibrateScale(float weight, uint8_t times)
    {
        float net = readAverage(times) - _offset;
        if (weight == 0 || net == 0)
            return;
        _scale = weight / net; // store COUNTS PER GRAM internally
        // getScale() will return 1/_scale = GRAMS PER COUNT
    }

private:
    PIO pio_ = pio0;
    uint sm_;

    static inline bool programLoaded = false;
    static inline uint programOffset = 0;

    uint clockPin_;
    uint dataPin_;
    int32_t _offset = 0;
    float _scale = 1.0f;
};

// === helpers ===
static void drain_input(void)
{
    while (stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
    { /* discard */
    }
}

// Read unsigned integer until Enter or timeout.
// If digits were typed but no Enter arrives before timeout, accept them.
static bool read_uint32_until_enter_or_timeout(uint32_t *out, uint32_t timeout_us)
{
    uint32_t value = 0;
    bool got_digit = false;
    absolute_time_t deadline = make_timeout_time_us(timeout_us);

    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0)
    {
        int ch = stdio_getchar_timeout_us(50 * 1000);
        if (ch == PICO_ERROR_TIMEOUT)
            continue;

        if (ch == '\r' || ch == '\n')
        {
            if (got_digit)
            {
                *out = value;
                return true;
            }
            return false; // Enter without digits
        }
        if (ch >= '0' && ch <= '9')
        {
            got_digit = true;
            value = value * 10u + (uint32_t)(ch - '0');
            putchar((char)ch); // echo
            fflush(stdout);
        }
        // ignore other chars
    }
    // timed out: accept if we saw digits
    if (got_digit)
    {
        *out = value;
        return true;
    }
    return false;
}

// Small helper: wait for any non-CR/LF key with timeout
static bool wait_any_key(uint32_t timeout_us)
{
    absolute_time_t deadline = make_timeout_time_us(timeout_us);
    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0)
    {
        int ch = stdio_getchar_timeout_us(50 * 1000);
        if (ch >= 0 && ch != '\r' && ch != '\n')
        {
            return true; // no drain here
        }
    }
    return false;
}

// // === main ===
// int main()
// {
//     stdio_init_all();
//     setvbuf(stdout, NULL, _IONBF, 0);
//     sleep_ms(300);

//     // Wait up to 5s for USB CDC; then continue anyway.
//     absolute_time_t d = make_timeout_time_ms(5000);
//     while (!stdio_usb_connected() && absolute_time_diff_us(get_absolute_time(), d) > 0)
//     {
//         sleep_ms(50);
//     }

//     printf("\nPico USB up (connected=%d)\n", stdio_usb_connected() ? 1 : 0);

//     HX711 myScale(16, 17);

//     for (;;)
//     {
//         // Flush any leftover keystrokes
//         while (stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
//         {
//         }

//         printf("\n\nCALIBRATION\n===========\n");

//         // 1) Remove weight and confirm (10s)
//         printf("remove all weight from the loadcell\n");
//         printf("press any key (10s timeout)...\n");
//         absolute_time_t deadline = make_timeout_time_us(10 * 1000 * 1000);
//         bool ok = false;
//         while (absolute_time_diff_us(get_absolute_time(), deadline) > 0)
//         {
//             int ch = stdio_getchar_timeout_us(100 * 1000);
//             if (ch >= 0 && ch != '\r' && ch != '\n')
//             {
//                 ok = true;
//                 break;
//             }
//         }
//         if (!ok)
//         {
//             printf("Timeout, restart.\n");
//             continue;
//         }

//         // 2) Tare (avg 20)
//         printf("Determine zero weight offset\n");
//         myScale.tare(20);
//         int32_t offset = myScale.getOffset();
//         printf("OFFSET: %ld\n\n", (long)offset);

//         // 3) Enter known weight (digits + Enter, 10s)
//         // Give yourself up to 100 s to place the weight
//         printf("Place the known weight, then press any key (100s timeout)...\n");
//         if (!wait_any_key(100 * 1000 * 1000))
//         {
//             printf("Timeout waiting to place weight.\n");
//             continue;
//         }

//         // IMPORTANT: now that the key is detected, prep for numeric entry
//         printf("Enter the known weight in grams (digits only), then press Enter (100s timeout): ");
//         // give the serial monitor a moment to finish that keypress packet, then clear leftovers
//         sleep_ms(30);
//         while (stdio_getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
//         {
//         } // drain ONCE here

//         uint32_t weight = 0;
//         if (!read_uint32_until_enter_or_timeout(&weight, 100 * 1000 * 1000))
//         {
//             printf("\nNo valid number entered.\n");
//             continue;
//         }
//         printf("\nWEIGHT: %u\n", weight);

//         myScale.calibrateScale((float)weight, 20);
//         printf("SCALE (g/count): %.6f\n", myScale.getScale());

//         // Now spit out 30 readings
//         for (int i = 0; i < 30; i++)
//         {
//             float grams = myScale.read();
//             printf("Weight: %.2f g\n", grams);
//             sleep_ms(200);
//         }
//     }
// }

int main()
{
    stdio_init_all();

    HX711 myScale(16, 17);

    while (true) {
        int32_t read = myScale.read();
        int32_t readWithOffset = read - 40937;
        float weight = readWithOffset / 280.0f;
        printf("Scale read: %d with Offset: %d weight: %.2f\n " ,read, readWithOffset, weight);
    }
}
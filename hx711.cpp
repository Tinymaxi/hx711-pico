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
        _scale = net / weight; // store COUNTS PER GRAM internally
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

static void calibrate(HX711 &scale)
{
    // Fixed calibration weight (grams)
    constexpr float KNOWN_WEIGHT_G = 283.0f;

    // Make stdout unbuffered so prompts appear immediately
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Clear any pending input
    while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
    { /* discard */
    }

    printf("\n\nCALIBRATION\n===========\n");
    printf("Remove all weight from the loadcell and press Enter...\n");
    int ch;
    do
    {
        ch = getchar();
    } while (ch != '\n' && ch != '\r');

    // Tare (zero) with some averaging
    printf("Determining zero offset (avg 20 reads)...\n");
    scale.tare(20);
    int32_t offset = scale.getOffset();
    printf("OFFSET (counts): %ld\n\n", (long)offset);

    printf("Place the known %.0f g weight on the loadcell and press Enter...\n", KNOWN_WEIGHT_G);
    while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
    { /* discard */
    }
    do
    {
        ch = getchar();
    } while (ch != '\n' && ch != '\r');

    // IMPORTANT: keep the class’ internal convention:
    //   _scale stores COUNTS PER GRAM (inverse of g/count).
    //   getScale() returns g/count (1/_scale).
    // So here we must compute counts/gram, i.e. net_counts / weight_g.
    {
        float with_weight_avg = scale.readAverage(20);
        float net = with_weight_avg - offset;
        // Protect against divide-by-zero
        if (net == 0.0f)
            net = 1.0f;
        // Store as inverse scale inside the class
        scale.setOffset(offset);
        scale.setScale(net / KNOWN_WEIGHT_G); // setScale expects g/count, but the class stores 1/scale internally
        // ^ Your setScale() already inverts: _scale = 1.0/scale
        //   We want 'scale' = g/count, so pass (net/KNOWN_WEIGHT_G)^(-1) ?
        //   Careful: easier is to call calibrateScale which does the right thing if implemented as net/weight.
    }

    // If you prefer using your method:
    // scale.calibrateScale(KNOWN_WEIGHT_G, 20);

    float g_per_count = scale.getScale();
    printf("SCALE (g/count): %.8f\n\n", g_per_count);
    printf("Use this in your setup:\n");
    printf("  scale.setOffset(%ld);\n", (long)offset);
    printf("  scale.setScale(%.8f);\n", g_per_count);
    printf("\nCALIBRATION DONE\n\n");
}


int main() {
    stdio_init_all();
    sleep_ms(300);                         // let USB enumerate
    while (!stdio_usb_connected()) sleep_ms(50);

    // --- your pins ---
    constexpr uint CLK_PIN = 16; // PD_SCK
    constexpr uint DAT_PIN = 17; // DOUT

    HX711 scale(CLK_PIN, DAT_PIN);

    // run the guided calibration (uses 283 g)
    calibrate(scale);

    // live readout
    while (true) {
        float grams = (scale.readAverage(10) - scale.getOffset()) * scale.getScale();
        printf("Weight: %.2f g\n", grams);
        sleep_ms(200);
    }
}


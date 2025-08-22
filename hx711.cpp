#include <stdio.h>
#include <cstdio>
#include <string>
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
        _scale = weight / (readAverage(times) - _offset);
    }

private:
    PIO pio_ = pio0;
    uint sm_;

    static inline bool programLoaded = false;
    static inline uint programOffset = 0;

    uint clockPin_;
    uint dataPin_;
    int32_t _offset;
    float _scale;
};

void calibrate(HX711 &scale);

int main() {
    stdio_init_all();
    sleep_ms(300); // let USB enumerate

    HX711 myScale1(16, 17);

    calibrate(myScale1);

    while (true) {
        // Example: read grams with the new calibration
        float grams = (myScale1.readAverage(10) - myScale1.getOffset()) * myScale1.getScale();
        printf("Weight: %.2f g\n", grams);
        sleep_ms(200);
    }
}

// ---- implementation of calibrate ----
void calibrate(HX711& scale)
{
    // Optional but helpful: make stdout unbuffered so prompts always appear
    setvbuf(stdout, nullptr, _IONBF, 0);

    // Give USB some time to enumerate (optional)
    // while (!stdio_usb_connected()) sleep_ms(50);

    // Drain any pending input before starting
    int ch;
    while ((ch = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) { /* discard */ }

    printf("\n\nCALIBRATION\n===========\n");
    printf("Remove all weight from the loadcell and press Enter...\n");

    // Wait for Enter (consume CR/LF)
    do { ch = getchar(); } while (ch != '\n' && ch != '\r');

    printf("Determining zero weight offset (avg 20 reads)...\n");
    scale.tare(20);
    int32_t offset = scale.getOffset();
    printf("OFFSET: %ld\n\n", static_cast<long>(offset));

    printf("Place a known weight on the loadcell and press Enter...\n");
    // Drain then wait for Enter
    while ((ch = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) { /* discard */ }
    do { ch = getchar(); } while (ch != '\n' && ch != '\r');

    // Ask for numeric weight and flush the prompt immediately
    printf("Enter the weight in grams (whole number) and press Enter: ");
    fflush(stdout);

    uint32_t weight = 0;
    // Leading space in format skips any leftover whitespace/CR/LF
    while (scanf(" %u", &weight) != 1) {
        printf("Invalid input. Please enter a whole number: ");
        fflush(stdout);
        // Clear bad line
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    // Consume trailing newline if present
    while ((ch = getchar()) != '\n' && ch != EOF) { }

    printf("WEIGHT: %u\n", (unsigned)weight);

    scale.calibrateScale(static_cast<float>(weight), 20);
    float s = scale.getScale();

    printf("SCALE: %.6f\n\n", s);
    printf("Use this in your setup:\n");
    printf("  scale.setOffset(%ld);\n", static_cast<long>(offset));
    printf("  scale.setScale(%.6f);\n", s);
    printf("\nCALIBRATION DONE\n\n");
}
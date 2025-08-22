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

int main()
{
    stdio_init_all();

    HX711 myScale1(16, 17);
    HX711 myScale2(18, 19);
    HX711 myScale3(20, 21);

    while (true)
    {
        // int32_t v1 = myScale1.read();
        // int32_t v2 = myScale2.read();
        // int32_t v3 = myScale3.read();

        // printf("my scale 1: %ld", v1);
        // printf(" my scale 2: %ld", v2);
        // printf(" my scale 3: %ld\n",v3);

        float a1 = myScale3.readAverage(20);
        printf("Average : %f\n", a1);
    }
}
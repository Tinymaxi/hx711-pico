#include "HX711.hpp"

#include <iostream>
#include "pico/time.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hx711_reader.pio.h"

HX711::HX711(uint clockPin, uint dataPin)
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
int32_t HX711::read_raw_hx711()
{
    uint32_t raw = pio_sm_get_blocking(pio_, sm_);

    // Sign-extend 24 → 32 bits
    if (raw & 0x00800000)
    {
        raw |= 0xFF000000;
    }

    return static_cast<int32_t>(raw);
}

float HX711::calibr_read_average(uint8_t times)
{

    uint32_t discardReads = 6; // Old reads left in the FIFO
    float sum = 0;
    for (uint8_t i = 0; i < times + discardReads; i++)
    {
        uint32_t v = read_raw_hx711();
        if (i >= discardReads)
        {
            printf("Read : %d -> %d\n", i, v);

            sum += v;
            printf("Sum of average reads: %.2f\n", sum);
        }
    }
    return sum / times;
}



///////////////////////////////////////////////////////
//
//  MODE
//
float HX711::read_weight()
{
    float weight = read_raw_hx711();
    return (weight - _offset) * _scale;
}

///////////////////////////////////////////////////////
//
//  TARE
//
void HX711::tare(uint8_t times)
{
    _offset = calibr_read_average(times);
}

float HX711::get_tare()
{
    return -_offset * _scale;
}

bool HX711::tare_set()
{
    return _offset != 0;
}

///////////////////////////////////////////////////////////////
//
//  CALIBRATION  (tare see above)
//
bool HX711::set_scale(float scale)
{
    if (scale == 0)
        return false;
    _scale = 1.0f / scale;
    return true;
}

float HX711::get_scale()
{
    return 1.0f / _scale;
}

void HX711::set_offset(int32_t offset)
{
    _offset = offset;
}

int32_t HX711::get_offset()
{
    return _offset;
}

//  assumes tare() has been set.
void HX711::calibrate_scale(float weight, uint8_t times)
{
    float net = calibr_read_average(times) - _offset;
    if (weight == 0 || net == 0)
        return;
    _scale = weight / net; // store COUNTS PER GRAM internally
    // getScale() will return 1/_scale = GRAMS PER COUNT
    printf("Scale : %f, net: %.2f\n", _scale, net);
}

///////////////////////////////////////////////////////////////
//
//  POWER MANAGEMENT
//
void HX711::power_down()
{
    // stop state machine so it won't fight us for the pin
    pio_sm_set_enabled(pio_, sm_, false);

    gpio_put(clockPin_, 1); // drive PD_SCK high
    sleep_us(64);           // ≥60 µs

    // leave it high; HX711 stays in power-down
}

void HX711::power_up()
{
    gpio_put(clockPin_, 0); // pull PD_SCK low
    sleep_us(1);            // small settle
    // re-enable the state machine when you want to read again
    pio_sm_set_enabled(pio_, sm_, true);
}
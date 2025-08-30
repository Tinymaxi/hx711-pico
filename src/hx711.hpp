#pragma once

#include <cstdint>
#include "hardware/pio.h"

class hx711
{
public:
    hx711(uint clockPin, uint dataPin);

    ///////////////////////////////////////////////////////////////
    //
    //  READ
    //
    //  From datasheet page 4
    int32_t read_raw_hx711();

    float   calibr_read_average(uint8_t times);

    ///////////////////////////////////////////////////////
    //
    //  MODE
    //
    float  read_weight(int samples = 1);

    ///////////////////////////////////////////////////////
    //
    //  TARE
    //
    void    tare(int samples = 10);
    float   get_tare();
    bool    tare_set();

    ///////////////////////////////////////////////////////////////
    //
    //  CALIBRATION  (tare see above)
    //
    void    set_scale(float counts_per_gram);
    float   get_scale() const;
    void    set_offset(int32_t offset);
    int32_t get_offset() const;

    //  clear the scale
    //  call tare() to set the zero offset
    //  put a known weight on the scale
    //  call calibrate_scale(weight)
    //  scale is calculated.
    //  Calibrate: known weight in grams, compute counts_per_gram
    void   calibrate_scale(float known_grams, int samples = 10);

    ///////////////////////////////////////////////////////////////
    //
    //  POWER MANAGEMENT
    //
    void    power_down();
    void    power_up();

private:
    PIO pio_ = pio0;
    uint sm_;

    static inline bool programLoaded = false;
    static inline uint programOffset = 0;

    uint    clockPin_;
    uint    dataPin_;
    float   scale_cpg_ { 1.0f }; // counts per gram
    int32_t offset_    { 0 };
};
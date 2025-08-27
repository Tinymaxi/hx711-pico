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
    float   read_weight();

    ///////////////////////////////////////////////////////
    //
    //  TARE
    //
    void    tare(uint8_t times);
    float   get_tare();
    bool    tare_set();

    ///////////////////////////////////////////////////////////////
    //
    //  CALIBRATION  (tare see above)
    //
    bool    set_scale(float scale);
    float   get_scale();
    void    set_offset(int32_t offset);
    int32_t get_offset();

    //  clear the scale
    //  call tare() to set the zero offset
    //  put a known weight on the scale
    //  call calibrate_scale(weight)
    //  scale is calculated.
    void calibrate_scale(float weight, uint8_t times);

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

    uint clockPin_;
    uint dataPin_;
    int32_t _offset = 0;
    float _scale = 1.0f;
};
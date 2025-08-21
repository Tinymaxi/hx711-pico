// MIT License
// 
// Copyright (c) 2023 Daniel Robertson
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

// This header provides a thin, zero-overhead C++ wrapper around the original
// C driver (hx711.h / hx711.c). The wrapper keeps ALL of the original code
// and comments in place; it simply constructs the C driver using the pins
// you pass to the constructor and exposes convenient C++ methods.

#include <cstdint>
#include "pico/stdlib.h"
#include "hardware/pio.h"

// Original C API (kept intact)
extern "C" {
#include "hx711.h"
}


class Hx711 {
public:
    // Constructor takes CLOCK_PIN and DATA_PIN (in that order), as requested.
    // Uses PIO0 by default, matching the common setup. If you need PIO1,
    // change it below in the implementation or add a third optional arg later.
    Hx711(uint clock_pin, uint data_pin);

    // Non-copyable (owns underlying state machine claim)
    Hx711(const Hx711&)            = delete;
    Hx711& operator=(const Hx711&) = delete;

    // Movable
    Hx711(Hx711&& other) noexcept;
    Hx711& operator=(Hx711&& other) noexcept;

    ~Hx711();

    // Thin wrappers over the original C API (names kept consistent):
    void     power_up(hx711_gain_t gain);
    void     power_down();
    static   void wait_settle(hx711_rate_t rate) { hx711_wait_settle(rate); }
    static   void wait_power_down()              { hx711_wait_power_down(); }

    void     set_gain(hx711_gain_t gain);
    int32_t  get_value();                         // blocking
    bool     get_value_timeout(int32_t* val, uint timeout_us);
    bool     get_value_noblock(int32_t* val);

    // Useful helpers, passed through:
    static   unsigned short get_settling_time(hx711_rate_t rate) { return hx711_get_settling_time(rate); }
    static   unsigned char  get_rate_sps(hx711_rate_t rate)      { return hx711_get_rate_sps(rate); }
    static   unsigned char  get_clock_pulses(hx711_gain_t gain)  { return hx711_get_clock_pulses(gain); }
    static   int32_t        get_twos_comp(uint32_t raw)          { return hx711_get_twos_comp(raw); }

    // Expose the underlying handle so C code (e.g. scale.c) can call the original API.
    hx711_t* handle() { return &_hx; }

private:
    hx711_t _hx{};   // original C driver handle (kept as-is)
    bool    _inited{false};

    void close_if_open();
};
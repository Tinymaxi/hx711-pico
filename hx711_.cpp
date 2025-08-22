// MIT License
// 
// Copyright (c) 2023 Daniel Robertson
// 
// (License text identical to header; kept verbatim to preserve attribution.)
// 
// This file wraps the original C implementation (hx711.c) without changing it.
// All original comments remain in the original files; here we only construct
// the C driver with the pins provided to the C++ constructor and forward calls.

#include "hx711.hpp"            // your C++ wrapper header
extern "C" {
  #include "hx711.h"            // the original C API
}
#include "hx711_reader.pio.h"

// ---- Construction / Destruction ------------------------------------------------

Hx711::Hx711(uint clock_pin, uint data_pin) {
    // Build the C config using your two pins, PIO0, and the PIO program symbols
    // provided by hx711_reader.pio (through hx711_reader.pio.h).
    hx711_config_t cfg{};
    cfg.clock_pin       = clock_pin;
    cfg.data_pin        = data_pin;
    cfg.pio             = pio0;                       // choose PIO0
    cfg.pio_init        = hx711_reader_pio_init;      // from the .pio c-sdk section
    cfg.reader_prog     = &hx711_reader_program;      // from the .pio generated header
    cfg.reader_prog_init= hx711_reader_program_init;  // from the .pio c-sdk section

    // Initialize the original C driver (kept intact)
    hx711_init(&_hx, &cfg);
    _inited = true;
}

Hx711::Hx711(Hx711&& other) noexcept {
    _hx     = other._hx;
    _inited = other._inited;
    other._inited = false;
}

Hx711& Hx711::operator=(Hx711&& other) noexcept {
    if (this != &other) {
        close_if_open();
        _hx     = other._hx;
        _inited = other._inited;
        other._inited = false;
    }
    return *this;
}

Hx711::~Hx711() {
    close_if_open();
}

void Hx711::close_if_open() {
    if (_inited) {
        // Close via the original C API (kept as-is)
        hx711_close(&_hx);
        _inited = false;
    }
}

// ---- Forwarders to the C API ---------------------------------------------------

void Hx711::power_up(hx711_gain_t gain) {
    hx711_power_up(&_hx, gain);
}

void Hx711::power_down() {
    hx711_power_down(&_hx);
}

void Hx711::set_gain(hx711_gain_t gain) {
    hx711_set_gain(&_hx, gain);
}

int32_t Hx711::get_value() {
    return hx711_get_value(&_hx);
}

bool Hx711::get_value_timeout(int32_t* val, uint timeout_us) {
    return hx711_get_value_timeout(&_hx, val, timeout_us);
}

bool Hx711::get_value_noblock(int32_t* val) {
    return hx711_get_value_noblock(&_hx, val);
}
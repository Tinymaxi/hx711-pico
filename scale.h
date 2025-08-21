#pragma once
#include <stddef.h>
#include <stdint.h>
#include "hx711.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t offset_counts;    // tare (empty platform) in counts
    float   g_per_count;      // grams per count (calibration factor)
    // filtering params
    size_t  N;                // window size
    size_t  k;                // trim each side (drop k lows & k highs)
} scale_t;

/** Initialize defaults for the filter (safe starting point). */
static inline void scale_init(scale_t* s) {
    s->offset_counts = 0;
    s->g_per_count   = 1.0f;
    s->N = 15;   // 10–20 samples is good at 10 SPS
    s->k = 3;    // drop 3 lowest & 3 highest
}

/** Collect N samples from HX711 and return a trimmed-mean (spike resistant). */
int32_t scale_read_counts_filtered(hx711_t* hx, const scale_t* s);

/** Tare: record current filtered counts as zero-load baseline. */
void scale_tare(hx711_t* hx, scale_t* s);

/** Calibrate with a known weight (grams). Assumes tare already done. */
int  scale_calibrate(hx711_t* hx, scale_t* s, float known_grams);

/** Read current weight in grams using current tare + calibration. */
float scale_read_grams(hx711_t* hx, const scale_t* s);

// -------- rolling trimmed-mean filter (15 window, drop 3 low/high) --------
void  filt_init(void);
void  filt_add(int32_t v);
float filt_get_trimmed_mean(void);

#ifdef __cplusplus
}
#endif
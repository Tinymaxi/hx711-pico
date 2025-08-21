#include "scale.h"
#include <stdlib.h>




static int cmp_i32(const void* a, const void* b) {
    int32_t x = *(const int32_t*)a, y = *(const int32_t*)b;
    return (x > y) - (x < y);
}

int32_t scale_read_counts_filtered(hx711_t* hx, const scale_t* s) {
    // Safety clamp for parameters
    size_t N = s->N ? s->N : 15;
    size_t k = s->k;
    if (N < 3) N = 3;
    if (2*k >= N) k = (N-1)/2;

    int32_t *buf = (int32_t*)malloc(N * sizeof(int32_t));
    if (!buf) return 0;

    for (size_t i = 0; i < N; ++i) {
        buf[i] = hx711_get_value(hx);  // blocking read @ ~10 SPS
    }

    qsort(buf, N, sizeof(int32_t), cmp_i32);

    int64_t sum = 0;
    size_t used = 0;
    for (size_t i = k; i < N - k; ++i) {
        sum += buf[i];
        used++;
    }
    free(buf);

    if (!used) return 0;
    return (int32_t)(sum / (int64_t)used);
}

void scale_tare(hx711_t* hx, scale_t* s) {
    s->offset_counts = scale_read_counts_filtered(hx, s);
}

int scale_calibrate(hx711_t* hx, scale_t* s, float known_grams) {
    int32_t with_weight = scale_read_counts_filtered(hx, s);
    int32_t net = with_weight - s->offset_counts;
    if (net == 0) return 0;              // failed (no delta)
    s->g_per_count = known_grams / (float)net;
    return 1;                             // ok
}

float scale_read_grams(hx711_t* hx, const scale_t* s) {
    int32_t filt = scale_read_counts_filtered(hx, s);
    int32_t net  = filt - s->offset_counts;
    return (float)net * s->g_per_count;
}

// ---- config ----
#define W 15
#define TRIM 3  // drop 3 low + 3 high

// ---- storage ----
static int32_t ring[W];
static int32_t sorted[W];
static size_t  ring_idx = 0;
static size_t  filled   = 0;

// helper: remove one value from sorted[]
static void sorted_remove(int32_t v) {
    size_t i = 0;
    while (i < filled && sorted[i] != v) i++;
    if (i == filled) return;                // shouldn't happen after warm-up
    for (; i + 1 < filled; ++i) sorted[i] = sorted[i+1];
    if (filled) filled--;
}

// helper: insert one value into sorted[] keeping order
static void sorted_insert(int32_t v) {
    size_t i = filled;
    while (i > 0 && sorted[i-1] > v) {
        sorted[i] = sorted[i-1];
        --i;
    }
    sorted[i] = v;
    filled++;
}

// call this once at boot
void filt_init(void) {
    ring_idx = 0; filled = 0;
}

// add a new sample (from hx711_get_value)
void filt_add(int32_t v) {
    if (filled < W) {
        // warming up
        ring[filled] = v;
        sorted_insert(v);
        ring_idx = (ring_idx + 1) % W;
    } else {
        // steady state: evict oldest, insert newest
        int32_t old = ring[ring_idx];
        ring[ring_idx] = v;
        ring_idx = (ring_idx + 1) % W;

        sorted_remove(old);
        sorted_insert(v);
    }
}

// get trimmed mean (drop TRIM low & TRIM high)
float filt_get_trimmed_mean(void) {
    if (filled == 0) return 0.0f;
    if (filled < 2*TRIM + 1) {
        // not enough for trimming yet: simple mean of what we have
        int64_t s = 0;
        for (size_t i = 0; i < filled; ++i) s += ring[i];
        return (float)s / (float)filled;
    }
    int64_t s = 0;
    for (size_t i = TRIM; i < filled - TRIM; ++i) s += sorted[i];
    size_t used = filled - 2*TRIM;
    return (float)s / (float)used;
}
#include <cstdio>
#include <cctype>
#include "pico/stdlib.h"
#if PICO_STDIO_USB
  #include "pico/stdio_usb.h"
#endif
#include "hx711.hpp"
extern "C" {
  #include "scale.h"
}

// ---------- helpers ----------
static void wait_for_serial()
{
  stdio_init_all();
#if PICO_STDIO_USB
  while (!stdio_usb_connected()) { sleep_ms(50); }
#endif
  sleep_ms(200); // let the host attach fully
  // flush any pre-existing input
  while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
}

static void wait_for_any_key(const char* prompt)
{
  printf("%s", prompt);
  while (true) {
    int ch = getchar_timeout_us(1000);
    if (ch == PICO_ERROR_TIMEOUT) continue;
    if (ch == '\n' || ch == '\r') continue; // ignore CR/LF
    printf("Got key: %c\n", ch);
    break;
  }
  // flush the rest
  while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
}

// Robust “read an unsigned integer” from serial:
//  - accepts digits 0–9
//  - finishes on Enter, any non-digit (e.g. space), or ~800 ms of idle
//  - echoes the digits so you can see what the monitor sent
static uint32_t read_uint_relaxed(const char* prompt)
{
  printf("%s", prompt);
  uint32_t v = 0;
  const uint32_t idle_us_to_finish = 800 * 1000; // finish if user pauses typing
  absolute_time_t idle_deadline = delayed_by_us(get_absolute_time(), idle_us_to_finish);

  while (true) {
    int ch = getchar_timeout_us(10 * 1000);
    if (ch == PICO_ERROR_TIMEOUT) {
      if (absolute_time_diff_us(get_absolute_time(), idle_deadline) <= 0) break;
      continue;
    }
    if (ch == '\n' || ch == '\r') break;
    if (std::isdigit(ch)) {
      v = v * 10u + (ch - '0');
      putchar(ch);           // echo digit
      idle_deadline = delayed_by_us(get_absolute_time(), idle_us_to_finish); // reset idle timer
    } else {
      // first non-digit ends entry (e.g. VS Code sending a space)
      break;
    }
  }
  printf("\n");
  // flush anything leftover on the line
  while (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT) {}
  return v;
}

// ---------- app ----------
int main()
{
  wait_for_serial();

  // Your wiring
  const uint CLOCK_PIN = 16;   // PD_SCK
  const uint DATA_PIN  = 17;   // DOUT

  Hx711 hx(CLOCK_PIN, DATA_PIN);
  hx.power_up(hx711_gain_128);
  Hx711::wait_settle(hx711_rate_10);

  scale_t sc;
  scale_init(&sc);   // N=15, k=3 by default (trimmed mean)

  // --- TARE ---
  wait_for_any_key("Remove all weight, then press any key to tare...\n");
  printf("Taring...\n");
  scale_tare(hx.handle(), &sc);
  printf("OFFSET (counts): %ld\n\n", (long)sc.offset_counts);

  // --- CALIBRATE ---
  wait_for_any_key("Place a known weight, then press any key when ready...\n");

  uint32_t grams = read_uint_relaxed(
    "Enter the weight in whole grams, then press Enter\n"
    "(or any non-digit, or pause ~0.8s after typing): "
  );
  if (grams == 0) {
    printf("No weight entered. Skipping calibration; using previous settings.\n");
  } else {
    // To make calibration snappy but still stable:
    size_t oldN = sc.N, oldK = sc.k;
    sc.N = 10;     // ~1s worth at 10 SPS
    sc.k = 3;      // keep trimming to fight spikes

    printf("Cal samples: N=%zu, k=%zu\n", sc.N, sc.k);
    int32_t with_weight = scale_read_counts_filtered(hx.handle(), &sc);
    int32_t net = with_weight - sc.offset_counts;
    printf("with_weight_counts: %ld, net_counts: %ld\n", (long)with_weight, (long)net);

    if (net == 0) {
      printf("Calibration FAILED (no delta). Try a heavier weight or check wiring.\n");
      // restore window
      sc.N = oldN; sc.k = oldK;
    } else {
      sc.g_per_count = (float)grams / (float)net;
      // restore window
      sc.N = oldN; sc.k = oldK;

      printf("Calibration OK.\n");
      printf("Use these in your project:\n");
      printf("  sc.offset_counts = %ld;\n", (long)sc.offset_counts);
      printf("  sc.g_per_count   = %.6f;\n\n", sc.g_per_count);

      // quick sanity check
      printf("Sanity check: grams/net_counts = %.6f g/count\n\n",
             (float)grams / (float)net);
    }
  }

  // --- Show a few readings (grams) ---
  for (int i = 0; i < 10; ++i) {
    int32_t filt = scale_read_counts_filtered(hx.handle(), &sc);
    int32_t net  = filt - sc.offset_counts;
    float grams_now = (float)net * sc.g_per_count;
    printf("Weight: %.2f g  (filt=%ld, net=%ld)\n", grams_now, (long)filt, (long)net);
    sleep_ms(500);
  }

  hx.power_down();
  Hx711::wait_power_down();
  printf("Done.\n");
  return 0;
}
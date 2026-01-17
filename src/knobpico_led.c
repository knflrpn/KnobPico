#include "knobpico_led.h"

#include "pico/stdlib.h"

#include "hardware/pio.h"

#include "ws2812.pio.h"

#include "knobpico_hw.h"
#include "knobpico_state.h"
#include "knobpico_time.h"
#include "pico/flash.h"

static inline uint8_t lerp_u8(uint8_t a, uint8_t b, float t) {
  return (uint8_t)(a + (float)(b - a) * t);
}

static inline uint32_t rgb_to_u32(BkRgb c) {
  // WS2812 expects GRB. The original firmware packed R into bits 8-15, G into 16-23.
  // That format is preserved here.
  return ((uint32_t)c.r << 8) | ((uint32_t)c.g << 16) | (uint32_t)c.b;
}

static uint32_t led_tick(BkLedConfig* led, bool pressed_or_macro, uint32_t now_ms) {
  if (pressed_or_macro) {
    led->progress = 0.0f;
    led->last_step_time_ms = now_ms;
    return rgb_to_u32(led->press_color);
  }

  if (led->seq_len == 0) return 0;

  uint32_t dt = now_ms - led->last_step_time_ms;
  led->last_step_time_ms = now_ms;

  if (led->fade_ms > 0) {
    led->progress += (float)dt / (float)led->fade_ms;
  } else {
    led->progress = 1.0f;
  }

  if (led->progress >= 1.0f) {
    led->progress = 0.0f;
    led->current_idx = (uint8_t)((led->current_idx + 1) % led->seq_len);
  }

  BkRgb c1 = led->sequence[led->current_idx];
  BkRgb c2 = led->sequence[(led->current_idx + 1) % led->seq_len];

  BkRgb out;
  out.r = lerp_u8(c1.r, c2.r, led->progress);
  out.g = lerp_u8(c1.g, c2.g, led->progress);
  out.b = lerp_u8(c1.b, c2.b, led->progress);

  return rgb_to_u32(out);
}

void core1_led_entry(void) {
  // Ensure that this core is safe for flash writes from other core
  bool safe = flash_safe_execute_core_init();
  while(!safe); // Die if safe initialization failed
//  multicore_lockout_victim_init();

  // Initialize the PIO for WS2812 control
  PIO pio = pio0;
  uint sm = 0;
  uint offset = pio_add_program(pio, &ws2812_program);
  ws2812_program_init(pio, sm, offset, NEOPIXEL_PIN, 800000, false);

  while (true) {
    KnobPicoState* st = knobpico_state();
    uint32_t now = millis();

    for (int i = 0; i < 3; i++) {
      bool pressed_or_macro = st->btn_rt[i].stable_state || st->macro_playing;
      uint32_t color = led_tick(&st->cfg.leds[i], pressed_or_macro, now);
      pio_sm_put_blocking(pio, sm, color << 8u);
    }

    // Allow some time for WS2812s to latch (and to not peg this core)
    sleep_ms(10);
  }
}

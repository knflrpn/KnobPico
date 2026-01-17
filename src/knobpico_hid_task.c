#include "knobpico_hid_task.h"

#include "pico/bootrom.h"

#include "tusb.h"

#include "knobpico_buttons.h"
#include "knobpico_encoder.h"
#include "knobpico_macro.h"
#include "knobpico_state.h"
#include "knobpico_time.h"

void hid_task(void) {
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  uint32_t now = millis();
  if (now - start_ms < interval_ms) return;
  start_ms += interval_ms;

  if (!tud_hid_ready()) return;

  KnobPicoState* st = knobpico_state();

  // 1) Macro playback
  if (st->macro_playing) {
    macro_process();
    return;
  }

  // 2) Three-button bootloader chord
  bool b0 = button_raw_is_pressed(0);
  bool b1 = button_raw_is_pressed(1);
  bool b2 = button_raw_is_pressed(2);
  bool all_held = b0 && b1 && b2;

  static uint32_t reset_start_ms = 0;
  if (all_held) {
    if (reset_start_ms == 0) reset_start_ms = now;
    else if (now - reset_start_ms > 1000) reset_usb_boot(0, 0);

    // Prevent triggering macros while chord is held
    for (int i = 0; i < 3; i++) st->btn_rt[i].long_press_handled = true;
  } else {
    reset_start_ms = 0;
  }

  // 3) Button processing
  if (!all_held) {
    for (uint8_t i = 0; i < 3; i++) {
      BkButtonEvent ev = button_update(i, now);
      if (ev == BTN_EVENT_SHORT) {
        macro_start(st->cfg.buttons[i].short_macro_id);
      } else if (ev == BTN_EVENT_LONG) {
        macro_start(st->cfg.buttons[i].long_macro_id);
      }
    }
  }

  // 4) Encoder -> scroll
  int16_t delta = encoder_read_delta();
  st->scroll_accumulator += delta;

  // Protect div-by-zero if host sets scroll_div to 0
  uint8_t div = st->cfg.global.scroll_div ? st->cfg.global.scroll_div : 1;
  int8_t scroll_clicks = (int8_t)(st->scroll_accumulator / div);

  if (scroll_clicks != 0) {
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 0, 0, scroll_clicks, 0);
    st->scroll_accumulator -= (int32_t)scroll_clicks * div;
  }
}

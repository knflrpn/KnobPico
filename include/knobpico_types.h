#pragma once

#include <stdint.h>
#include <stdbool.h>

// -------------------------
// Config protocol / storage
// -------------------------
#define CONFIG_SIZE_MAX  1024
#define MAX_MACROS       7   // 0 reserved as empty
#define MAX_MACRO_STEPS  32  // Button macro length
#define MAX_SEQ_LEN      8   // LED sequence

// -------------------------
// TLV Types
// -------------------------
typedef enum {
  TLV_GLOBAL_SETTINGS = 0x0001,
  TLV_BUTTON_MAP      = 0x0100,
  TLV_MACRO_DEF       = 0x0200,
  TLV_LED_CONFIG      = 0x0300,
} BkTlvType;

// -------------------------
// RGB / LED config
// -------------------------
typedef struct {
  uint8_t r, g, b;
} BkRgb;

typedef struct {
  // Pressed color
  BkRgb press_color;

  // Idle sequence colors
  BkRgb    sequence[MAX_SEQ_LEN];
  uint8_t  seq_len;
  uint16_t fade_ms;

  // Internal animation state
  uint8_t  current_idx;
  uint32_t last_step_time_ms;
  float    progress;
} BkLedConfig;

// -------------------------
// Macro / input config
// -------------------------
typedef struct {
  uint8_t  modifiers;
  uint8_t  keycode;
  uint16_t duration_ms;
} BkMacroStep;

typedef struct {
  uint16_t debounce_ms;
  uint16_t long_press_ms;
  uint8_t  scroll_div;
  int8_t   scroll_dir;
} BkGlobalSettings;

typedef struct {
  uint8_t pin;
  uint8_t short_macro_id;
  uint8_t long_macro_id;
} BkButtonMap;

typedef struct {
  // Single source of truth for device behavior.
  BkGlobalSettings global;
  BkMacroStep      macros[MAX_MACROS][MAX_MACRO_STEPS];
  BkLedConfig      leds[3];
  BkButtonMap      buttons[3];
} KnobPicoConfig;

// -------------------------
// Button runtime state
// -------------------------
typedef struct {
  bool     stable_state;         // debounced state (pressed = true)
  bool     last_reading;         // raw sample
  uint32_t last_debounce_time_ms;
  uint32_t press_start_time_ms;
  bool     long_press_handled;
} BkButtonRuntime;

typedef enum {
  BTN_EVENT_NONE = 0,
  BTN_EVENT_SHORT,
  BTN_EVENT_LONG,
} BkButtonEvent;

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "knobpico_types.h"
#include "usb_transport.h"

typedef struct {
  // Configuration and persistent blobs
  KnobPicoConfig cfg;
  uint8_t active_blob[CONFIG_SIZE_MAX];
  uint8_t staging_blob[CONFIG_SIZE_MAX];

  // Button runtime
  BkButtonRuntime btn_rt[3];

  // Encoder runtime
  uint16_t last_encoder_raw;
  bool encoder_initialized;
  int32_t scroll_accumulator;

  // Macro engine runtime
  uint8_t current_macro_id;
  int current_macro_step;
  uint32_t macro_next_time_ms;
  bool macro_playing;

  // Last feature-report response for the host
  BkTransportPacket last_response;
} KnobPicoState;

// Access the singleton runtime state.
KnobPicoState* knobpico_state(void);

// Initializes the singleton state with factory defaults and populates active_blob.
void knobpico_state_init(uint32_t now_ms);

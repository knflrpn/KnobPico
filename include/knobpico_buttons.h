#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "knobpico_types.h"

// Raw input (active low). Uses configured pins from KnobPicoState->cfg.buttons[].
bool button_raw_is_pressed(uint8_t idx);

// Debounced update function (returns SHORT/LONG events, or NONE).
BkButtonEvent button_update(uint8_t idx, uint32_t now_ms);

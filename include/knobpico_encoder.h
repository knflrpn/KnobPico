#pragma once

#include <stdint.h>

// Returns signed delta since last read, already applying scroll_dir.
int16_t encoder_read_delta(void);

// Optional encoder configuration step (hysteresis settings for AS5600-like devices).
void encoder_configure_hysteresis(void);

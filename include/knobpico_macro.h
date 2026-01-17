#pragma once

#include <stdint.h>

// Start macro playback by ID (0 = no-op).
void macro_start(uint8_t macro_id);

// Run macro playback state machine (call periodically).
void macro_process(void);

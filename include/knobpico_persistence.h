#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "knobpico_config.h"

// Load a previously-saved blob from flash into dst (and validate header+crc).
// Returns true if a valid config was loaded.
bool knobpico_persistence_load(uint8_t *dst, uint16_t cap);

// Save the blob (len bytes) into flash.
// Returns true on success.
bool knobpico_persistence_save(const uint8_t *blob, uint16_t len);

#pragma once

#include <stdint.h>
#include "pico/time.h"

static inline uint32_t millis(void) {
  return to_ms_since_boot(get_absolute_time());
}

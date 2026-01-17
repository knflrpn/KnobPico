#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct __attribute__((packed)) {
  uint16_t type;
  uint16_t len;
} BkTlvHeader;

bool tlv_write(uint8_t* blob, uint16_t cap, uint16_t* woff, uint16_t type,
               const void* payload, uint16_t len);

bool tlv_next(const uint8_t* blob, uint16_t total_len,
              uint16_t* roff, BkTlvHeader* out_hdr, const uint8_t** out_val);

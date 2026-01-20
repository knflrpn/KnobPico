#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "knobpico_types.h"

// Config blob header
#define CFG_MAGIC 0x424B4E50u // "BKNP"
#define CFG_VERSION 0x0001u

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint16_t version;
	uint16_t total_len;
	uint32_t crc32;
} BkConfigHeader;

void knobpico_config_set_factory_defaults(KnobPicoConfig *cfg, uint32_t now_ms);

uint16_t knobpico_config_serialize(const KnobPicoConfig *cfg, uint8_t *blob, uint16_t cap);

bool knobpico_config_apply(KnobPicoConfig *cfg,
						   const uint8_t *blob,
						   uint16_t max_len,
						   uint32_t now_ms);

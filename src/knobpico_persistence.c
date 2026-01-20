#include "knobpico_persistence.h"

#include <string.h>

#include "hardware/flash.h"
#include "pico/flash.h"
#include "hardware/sync.h"
#include "pico/multicore.h"
#include "pico/platform.h" // PICO_FLASH_SIZE_BYTES, XIP_BASE

#include "crc32.h"

// Use one sector at the very end of flash.
// Make sure nothing else uses this (no filesystem / no second image / etc).
#ifndef PICO_FLASH_SIZE_BYTES
// Fallback if not provided by your build; most Pico boards are 2MB.
// Prefer having PICO_FLASH_SIZE_BYTES defined by the SDK/board.
#define PICO_FLASH_SIZE_BYTES (2u * 1024u * 1024u)
#endif

#define PERSIST_SECTOR_SIZE FLASH_SECTOR_SIZE
#define PERSIST_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - PERSIST_SECTOR_SIZE)

static const uint8_t *flash_ptr(void)
{
	return (const uint8_t *)(XIP_BASE + PERSIST_FLASH_OFFSET);
}

bool knobpico_persistence_load(uint8_t *dst, uint16_t cap)
{
	if (!dst || cap < sizeof(BkConfigHeader))
		return false;

	const uint8_t *src = flash_ptr();

	BkConfigHeader hdr;
	memcpy(&hdr, src, sizeof(hdr));

	if (hdr.magic != CFG_MAGIC)
		return false;
	if (hdr.version != CFG_VERSION)
		return false;
	if (hdr.total_len < sizeof(BkConfigHeader))
		return false;
	if (hdr.total_len > cap)
		return false;
	if (hdr.total_len > PERSIST_SECTOR_SIZE)
		return false;

	uint32_t calc = crc32_calc(src + sizeof(BkConfigHeader),
							   hdr.total_len - sizeof(BkConfigHeader));
	if (calc != hdr.crc32)
		return false;

	// Copy the valid blob out into RAM
	memcpy(dst, src, hdr.total_len);

	// Zero remaining bytes
	if (hdr.total_len < cap)
	{
		memset(dst + hdr.total_len, 0, cap - hdr.total_len);
	}

	return true;
}

bool knobpico_persistence_save(const uint8_t *blob, uint16_t len)
{
	// Sanity checks
	if (!blob || len < sizeof(BkConfigHeader))
		return false;
	if (len > PERSIST_SECTOR_SIZE)
		return false;

	// Confirm that it's safe to operate on flash
	flash_safety_helper_t *helper = get_flash_safety_helper();
	if (!helper)
		return false;

	int rc = helper->enter_safe_zone_timeout_ms(10);
	if (!rc)
	{

		// Flash programming must be page aligned. We'll write a full sector.
		static uint8_t sector_buf[PERSIST_SECTOR_SIZE] __attribute__((aligned(4)));

		memset(sector_buf, 0xFF, sizeof(sector_buf));
		memcpy(sector_buf, blob, len);

		flash_range_erase(PERSIST_FLASH_OFFSET, PERSIST_SECTOR_SIZE);
		flash_range_program(PERSIST_FLASH_OFFSET, sector_buf, PERSIST_SECTOR_SIZE);

		rc = helper->exit_safe_zone_timeout_ms(10);
	}

	return true;
}

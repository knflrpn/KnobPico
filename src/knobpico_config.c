#include "knobpico_config.h"

#include <string.h>

#include "knobpico_hw.h"
#include "crc32.h"
#include "tlv.h"
#include "tusb.h" // HID_KEY_* / KEYBOARD_MODIFIER_* constants

void knobpico_config_set_factory_defaults(KnobPicoConfig *cfg, uint32_t now_ms)
{
	if (!cfg)
		return;
	memset(cfg, 0, sizeof(*cfg));

	cfg->global.debounce_ms = DEFAULT_DEBOUNCE_MS;
	cfg->global.long_press_ms = DEFAULT_LONG_PRESS_MS;
	cfg->global.scroll_div = DEFAULT_SCROLL_DIV;
	cfg->global.scroll_dir = DEFAULT_SCROLL_DIR;

	// Default macros
	// IDs: 1/2 left, 3/4 mid, 5/6 right, 0 empty
	cfg->macros[1][0] = (BkMacroStep){KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_X, 25};
	cfg->macros[3][0] = (BkMacroStep){KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_C, 25};
	cfg->macros[5][0] = (BkMacroStep){KEYBOARD_MODIFIER_LEFTCTRL, HID_KEY_V, 25};

	// Default button mappings
	cfg->buttons[0] = (BkButtonMap){.pin = PIN_LEFT, .short_macro_id = 1, .long_macro_id = 2};
	cfg->buttons[1] = (BkButtonMap){.pin = PIN_MID, .short_macro_id = 3, .long_macro_id = 4};
	cfg->buttons[2] = (BkButtonMap){.pin = PIN_RIGHT, .short_macro_id = 5, .long_macro_id = 6};

	// Default LEDs
	cfg->leds[0] = (BkLedConfig){
		.press_color = {255, 255, 255},
		.sequence = {{0, 0, 50}, {0, 100, 200}},
		.seq_len = 2,
		.fade_ms = 1000};
	cfg->leds[1] = (BkLedConfig){
		.press_color = {255, 255, 255},
		.sequence = {{50, 0, 50}, {150, 0, 150}},
		.seq_len = 2,
		.fade_ms = 800};
	cfg->leds[2] = (BkLedConfig){
		.press_color = {255, 255, 255},
		.sequence = {{0, 20, 0}, {0, 150, 0}},
		.seq_len = 2,
		.fade_ms = 1200};
}

uint16_t knobpico_config_serialize(const KnobPicoConfig *cfg, uint8_t *blob, uint16_t cap)
{
	if (!cfg || !blob)
		return 0;
	if (cap < sizeof(BkConfigHeader))
		return 0;

	memset(blob, 0, cap);
	uint16_t woff = (uint16_t)sizeof(BkConfigHeader);

	// Global settings TLV: [debounce(2), long_press(2), scroll_div(1), scroll_dir(1)]
	uint8_t global_data[6];
	memcpy(&global_data[0], &cfg->global.debounce_ms, 2);
	memcpy(&global_data[2], &cfg->global.long_press_ms, 2);
	global_data[4] = cfg->global.scroll_div;
	global_data[5] = (uint8_t)cfg->global.scroll_dir;
	(void)tlv_write(blob, cap, &woff, TLV_GLOBAL_SETTINGS, global_data, sizeof(global_data));

	// Macros TLV: [id(1), count(1), steps...]
	for (uint8_t mid = 0; mid < MAX_MACROS; mid++)
	{
		uint8_t count = 0;
		for (int s = 0; s < MAX_MACRO_STEPS; s++)
		{
			const BkMacroStep *st = &cfg->macros[mid][s];
			if (st->duration_ms == 0 && st->keycode == 0 && st->modifiers == 0)
				break;
			count++;
		}
		if (count == 0)
			continue;

		uint8_t buf[2 + sizeof(BkMacroStep) * MAX_MACRO_STEPS];
		buf[0] = mid;
		buf[1] = count;
		memcpy(&buf[2], cfg->macros[mid], (size_t)count * sizeof(BkMacroStep));
		(void)tlv_write(blob, cap, &woff, TLV_MACRO_DEF, buf,
						(uint16_t)(2 + (uint16_t)count * sizeof(BkMacroStep)));
	}

	// LEDs TLV: [id(1), press_rgb(3), seq_len(1), fade_ms(2), seq_rgb...]
	for (uint8_t i = 0; i < 3; i++)
	{
		uint8_t led_buf[1 + 3 + 1 + 2 + 3 * MAX_SEQ_LEN];
		led_buf[0] = i;
		memcpy(&led_buf[1], &cfg->leds[i].press_color, 3);
		led_buf[4] = cfg->leds[i].seq_len;
		memcpy(&led_buf[5], &cfg->leds[i].fade_ms, 2);

		uint8_t seq_len = cfg->leds[i].seq_len;
		if (seq_len > MAX_SEQ_LEN)
			seq_len = MAX_SEQ_LEN;
		memcpy(&led_buf[7], cfg->leds[i].sequence, (size_t)seq_len * sizeof(BkRgb));

		uint16_t tlv_len = (uint16_t)(7 + (uint16_t)seq_len * sizeof(BkRgb));
		(void)tlv_write(blob, cap, &woff, TLV_LED_CONFIG, led_buf, tlv_len);
	}

	// Button map TLV: [id(1), short_mid(1), long_mid(1)]
	for (uint8_t i = 0; i < 3; i++)
	{
		uint8_t map_buf[3] = {i, cfg->buttons[i].short_macro_id, cfg->buttons[i].long_macro_id};
		(void)tlv_write(blob, cap, &woff, TLV_BUTTON_MAP, map_buf, sizeof(map_buf));
	}

	// Header
	BkConfigHeader *hdr = (BkConfigHeader *)blob;
	hdr->magic = CFG_MAGIC;
	hdr->version = CFG_VERSION;
	hdr->total_len = woff;
	hdr->crc32 = crc32_calc(blob + sizeof(BkConfigHeader), woff - sizeof(BkConfigHeader));

	return woff;
}

bool knobpico_config_apply(KnobPicoConfig *cfg,
						   const uint8_t *blob,
						   uint16_t max_len,
						   uint32_t now_ms)
{
	if (!cfg || !blob)
		return false;
	if (max_len < sizeof(BkConfigHeader))
		return false;

	BkConfigHeader hdr;
	memcpy(&hdr, blob, sizeof(hdr));

	if (hdr.magic != CFG_MAGIC)
		return false;
	if (hdr.version != CFG_VERSION)
		return false;
	if (hdr.total_len > max_len || hdr.total_len < sizeof(BkConfigHeader))
		return false;

	uint32_t calc = crc32_calc(blob + sizeof(BkConfigHeader), hdr.total_len - sizeof(BkConfigHeader));
	if (calc != hdr.crc32)
		return false;

	// Start from current config to allow partial updates.
	uint16_t roff = (uint16_t)sizeof(BkConfigHeader);
	BkTlvHeader tlv;
	const uint8_t *val;

	while (tlv_next(blob, hdr.total_len, &roff, &tlv, &val))
	{
		switch ((BkTlvType)tlv.type)
		{
		case TLV_GLOBAL_SETTINGS:
		{
			if (tlv.len < 6)
				break;
			memcpy(&cfg->global.debounce_ms, &val[0], 2);
			memcpy(&cfg->global.long_press_ms, &val[2], 2);
			cfg->global.scroll_div = val[4];
			cfg->global.scroll_dir = (int8_t)val[5];
			break;
		}

		case TLV_MACRO_DEF:
		{
			if (tlv.len < 2)
				break;
			uint8_t mid = val[0];
			uint8_t count = val[1];
			uint16_t required = (uint16_t)(2 + (uint16_t)count * sizeof(BkMacroStep));
			if (mid >= MAX_MACROS || count > MAX_MACRO_STEPS || tlv.len < required)
				break;
			memset(cfg->macros[mid], 0, sizeof(cfg->macros[mid]));
			memcpy(cfg->macros[mid], val + 2, (size_t)count * sizeof(BkMacroStep));
			break;
		}

		case TLV_LED_CONFIG:
		{
			// [id(1), press_rgb(3), seq_len(1), fade_ms(2), seq_rgb...]
			if (tlv.len < 7)
				break;
			uint8_t lid = val[0];
			if (lid >= 3)
				break;

			memcpy(&cfg->leds[lid].press_color, val + 1, 3);
			cfg->leds[lid].seq_len = val[4];
			memcpy(&cfg->leds[lid].fade_ms, val + 5, 2);

			uint8_t seq_len = cfg->leds[lid].seq_len;
			if (seq_len > MAX_SEQ_LEN)
				seq_len = MAX_SEQ_LEN;
			uint16_t need = (uint16_t)(7 + (uint16_t)seq_len * sizeof(BkRgb));
			if (tlv.len < need)
				break;

			memcpy(cfg->leds[lid].sequence, val + 7, (size_t)seq_len * sizeof(BkRgb));
			cfg->leds[lid].seq_len = seq_len;

			// Reset animation state
			cfg->leds[lid].current_idx = 0;
			cfg->leds[lid].progress = 0.0f;
			cfg->leds[lid].last_step_time_ms = now_ms;
			break;
		}

		case TLV_BUTTON_MAP:
		{
			if (tlv.len < 3)
				break;
			uint8_t bid = val[0];
			if (bid >= 3)
				break;
			cfg->buttons[bid].short_macro_id = val[1];
			cfg->buttons[bid].long_macro_id = val[2];
			break;
		}

		default:
			// Unknown TLV: ignore for forward compatibility
			break;
		}
	}

	return true;
}

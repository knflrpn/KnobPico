#include "knobpico_encoder.h"

#include "hardware/i2c.h"

#include "knobpico_hw.h"
#include "knobpico_state.h"

int16_t encoder_read_delta(void)
{
	KnobPicoState *st = knobpico_state();

	uint8_t reg = 0x0E;
	uint8_t buf[2];

	i2c_write_blocking(I2C_PORT, ENCODER_ADDR, &reg, 1, true);
	i2c_read_blocking(I2C_PORT, ENCODER_ADDR, buf, 2, false);

	uint16_t current_raw = (uint16_t)((buf[0] << 8) | buf[1]);

	if (!st->encoder_initialized)
	{
		st->last_encoder_raw = current_raw;
		st->encoder_initialized = true;
		return 0;
	}

	int32_t diff = (int32_t)current_raw - (int32_t)st->last_encoder_raw;

	// 12-bit wraparound
	if (diff < -2048)
		diff += 4096;
	else if (diff > 2048)
		diff -= 4096;

	st->last_encoder_raw = current_raw;

	return (int16_t)(diff * st->cfg.global.scroll_dir);
}

void encoder_configure_hysteresis(void)
{
	// Matches the original firmware's register write.
	// [0x07]=CONF register, value=0x000C
	uint8_t config_data[3] = {0x07, 0x00, 0x0C};
	i2c_write_blocking(I2C_PORT, ENCODER_ADDR, config_data, 3, false);
}

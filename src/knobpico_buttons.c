#include "knobpico_buttons.h"
#include "hardware/gpio.h"
#include "knobpico_state.h"

bool button_raw_is_pressed(uint8_t idx)
{
	KnobPicoState *st = knobpico_state();
	if (idx >= 3)
		return false;
	// Active-low input
	return !gpio_get(st->cfg.buttons[idx].pin);
}

BkButtonEvent button_update(uint8_t idx, uint32_t now_ms)
{
	KnobPicoState *st = knobpico_state();
	if (idx >= 3)
		return BTN_EVENT_NONE;

	BkButtonRuntime *rt = &st->btn_rt[idx];

	bool reading = button_raw_is_pressed(idx);

	// Debounce edge tracking
	if (reading != rt->last_reading)
	{
		rt->last_debounce_time_ms = now_ms;
	}
	rt->last_reading = reading;

	// Debounced stable state transitions
	if ((now_ms - rt->last_debounce_time_ms) > st->cfg.global.debounce_ms)
	{
		if (reading != rt->stable_state)
		{
			rt->stable_state = reading;

			if (reading)
			{
				// pressed
				rt->press_start_time_ms = now_ms;
				rt->long_press_handled = false;
			}
			else
			{
				// released
				uint32_t hold = now_ms - rt->press_start_time_ms;
				if (hold < st->cfg.global.long_press_ms && !rt->long_press_handled)
				{
					return BTN_EVENT_SHORT;
				}
			}
		}

		// Long press while held
		if (rt->stable_state && !rt->long_press_handled)
		{
			uint32_t hold = now_ms - rt->press_start_time_ms;
			if (hold >= st->cfg.global.long_press_ms)
			{
				rt->long_press_handled = true;
				return BTN_EVENT_LONG;
			}
		}
	}

	return BTN_EVENT_NONE;
}

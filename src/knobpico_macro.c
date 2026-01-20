#include "knobpico_macro.h"

#include "tusb.h"

#include "knobpico_state.h"
#include "knobpico_time.h"

void macro_start(uint8_t macro_id)
{
	KnobPicoState *st = knobpico_state();
	if (macro_id == 0 || macro_id >= MAX_MACROS)
		return;

	const BkMacroStep *first = &st->cfg.macros[macro_id][0];
	if (first->duration_ms == 0 && first->keycode == 0 && first->modifiers == 0)
		return;

	st->current_macro_id = macro_id;
	st->current_macro_step = 0;
	st->macro_next_time_ms = millis();
	st->macro_playing = true;
}

void macro_process(void)
{
	KnobPicoState *st = knobpico_state();
	if (!st->macro_playing)
		return;

	uint32_t now = millis();
	if (now < st->macro_next_time_ms)
		return;

	// Check for max steps.
	// If limit was reached in the previous pass, the wait is now over
	// for that step's duration. Release and stop.
	if (st->current_macro_step >= MAX_MACRO_STEPS)
	{
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
		st->macro_playing = false;
		return;
	}

	BkMacroStep step = st->cfg.macros[st->current_macro_id][st->current_macro_step];

	// End of macro (Explicit empty step found)
	if (step.duration_ms == 0 && step.keycode == 0 && step.modifiers == 0)
	{
		tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
		st->macro_playing = false;
		return;
	}

	uint8_t key_array[6] = {0};
	if (step.keycode)
		key_array[0] = step.keycode;

	tud_hid_keyboard_report(REPORT_ID_KEYBOARD, step.modifiers, key_array);

	st->macro_next_time_ms = now + step.duration_ms;
	st->current_macro_step++;
}

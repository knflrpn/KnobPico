#include "knobpico_state.h"

#include <string.h>

#include "knobpico_config.h"
#include "knobpico_persistence.h"

static KnobPicoState g_state;

KnobPicoState *knobpico_state(void)
{
	return &g_state;
}

void knobpico_state_init(uint32_t now_ms)
{
	memset(&g_state, 0, sizeof(g_state));

	// Seed with defaults so config from flash can be partial.
	knobpico_config_set_factory_defaults(&g_state.cfg, now_ms);

	bool loaded = knobpico_persistence_load(g_state.active_blob, CONFIG_SIZE_MAX);

	if (loaded)
	{
		bool ok = knobpico_config_apply(&g_state.cfg, g_state.active_blob,
										CONFIG_SIZE_MAX, now_ms);
		if (!ok)
		{
			loaded = false;
			knobpico_config_set_factory_defaults(&g_state.cfg, now_ms);
		}
	}

	// Always regenerate a canonical blob from cfg
	(void)knobpico_config_serialize(&g_state.cfg,
									g_state.active_blob,
									CONFIG_SIZE_MAX);

	memcpy(g_state.staging_blob, g_state.active_blob, CONFIG_SIZE_MAX);
}

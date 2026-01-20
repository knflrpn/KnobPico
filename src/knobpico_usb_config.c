#include "knobpico_usb_config.h"

#include <string.h>

#include "knobpico_config.h"
#include "knobpico_state.h"
#include "knobpico_time.h"
#include "knobpico_persistence.h"

void usb_config_handle_command(BkTransportPacket *pkt)
{
	if (!pkt)
		return;

	KnobPicoState *st = knobpico_state();
	pkt->status = 0;

	switch ((BkOpCode)pkt->op)
	{
	case OP_GET_INFO:
	{
		// Returns first 8 bytes of header for host discovery.
		BkConfigHeader *hdr = (BkConfigHeader *)st->active_blob;
		memcpy(pkt->payload, hdr, 8);
		pkt->len = 8;
		break;
	}

	case OP_READ:
	{
		if (pkt->offset >= CONFIG_SIZE_MAX)
		{
			pkt->status = 1;
			pkt->len = 0;
			break;
		}

		uint16_t to_copy = pkt->len;
		if ((uint32_t)pkt->offset + to_copy > CONFIG_SIZE_MAX)
		{
			to_copy = (uint16_t)(CONFIG_SIZE_MAX - pkt->offset);
		}

		memcpy(pkt->payload, &st->active_blob[pkt->offset], to_copy);
		pkt->len = (uint8_t)to_copy;
		break;
	}

	case OP_WRITE:
	{
		if ((uint32_t)pkt->offset + pkt->len > CONFIG_SIZE_MAX)
		{
			pkt->status = 1;
			pkt->len = 0;
			break;
		}

		memcpy(&st->staging_blob[pkt->offset], pkt->payload, pkt->len);
		pkt->len = 0;
		break;
	}

	case OP_COMMIT:
	{
		bool ok = knobpico_config_apply(&st->cfg, st->staging_blob, CONFIG_SIZE_MAX, millis());
		pkt->status = ok ? 0 : 2;
		pkt->len = 0;

		if (ok)
		{
			// Update the active blob in RAM
			uint16_t len = knobpico_config_serialize(&st->cfg, st->active_blob, CONFIG_SIZE_MAX);
			// Persist to flash
			(void)knobpico_persistence_save(st->active_blob, len);
			// Sync Staging
			memcpy(st->staging_blob, st->active_blob, CONFIG_SIZE_MAX);
		}
		break;
	}

	case OP_LOAD_DEFAULTS:
	{
		knobpico_config_set_factory_defaults(&st->cfg, millis());
		(void)knobpico_config_serialize(&st->cfg, st->active_blob, CONFIG_SIZE_MAX);
		memcpy(st->staging_blob, st->active_blob, CONFIG_SIZE_MAX);
		pkt->len = 0;
		break;
	}

	default:
		pkt->status = 0xFF;
		pkt->len = 0;
		break;
	}
}

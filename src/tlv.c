#include "tlv.h"

#include <string.h>

bool tlv_write(uint8_t *blob, uint16_t cap, uint16_t *woff, uint16_t type,
			   const void *payload, uint16_t len)
{
	if (!blob || !woff)
		return false;
	if ((uint32_t)(*woff) + sizeof(BkTlvHeader) + len > cap)
		return false;

	BkTlvHeader hdr = {.type = type, .len = len};
	memcpy(&blob[*woff], &hdr, sizeof(hdr));
	*woff += (uint16_t)sizeof(hdr);

	if (len && payload)
	{
		memcpy(&blob[*woff], payload, len);
		*woff += len;
	}

	return true;
}

bool tlv_next(const uint8_t *blob, uint16_t total_len,
			  uint16_t *roff, BkTlvHeader *out_hdr, const uint8_t **out_val)
{
	if (!blob || !roff || !out_hdr || !out_val)
		return false;
	if (*roff + sizeof(BkTlvHeader) > total_len)
		return false;

	memcpy(out_hdr, &blob[*roff], sizeof(BkTlvHeader));
	uint16_t val_off = (uint16_t)(*roff + sizeof(BkTlvHeader));

	if ((uint32_t)val_off + out_hdr->len > total_len)
		return false;

	*out_val = &blob[val_off];
	*roff = (uint16_t)(val_off + out_hdr->len);
	return true;
}

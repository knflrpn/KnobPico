#include "crc32.h"

uint32_t crc32_calc(const uint8_t *data, size_t length)
{
	uint32_t crc = 0xFFFFFFFFu;
	for (size_t i = 0; i < length; i++)
	{
		crc ^= data[i];
		for (int j = 0; j < 8; j++)
		{
			if (crc & 1u)
				crc = (crc >> 1) ^ 0xEDB88320u;
			else
				crc >>= 1;
		}
	}
	return ~crc;
}

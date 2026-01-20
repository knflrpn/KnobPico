#pragma once

#include <stdint.h>

// -------------------------
// USB / HID Report IDs
// -------------------------
#define REPORT_ID_KEYBOARD 1
#define REPORT_ID_MOUSE 2
#define REPORT_ID_CONFIG 3

// -------------------------
// Config protocol operations (feature report)
// -------------------------
typedef enum
{
	OP_GET_INFO = 0x01,
	OP_READ = 0x02,
	OP_WRITE = 0x03,
	OP_COMMIT = 0x04,
	OP_LOAD_DEFAULTS = 0x05,
} BkOpCode;

typedef struct __attribute__((packed))
{
	uint8_t op;
	uint8_t seq;
	uint8_t status;
	uint16_t offset;
	uint8_t len;
	uint8_t flags;
	uint8_t payload[56];
} BkTransportPacket;

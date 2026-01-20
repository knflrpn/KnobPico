#pragma once

#include "usb_transport.h"

// Applies a config feature-report command to the device.
// The packet is modified in-place with status/len/payload for the response.
void usb_config_handle_command(BkTransportPacket *pkt);

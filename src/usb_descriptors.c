#include "tusb.h"

// Report IDs for the composite device
#define REPORT_ID_KEYBOARD 1
#define REPORT_ID_MOUSE 2
#define REPORT_ID_CONFIG 3

// Standard Device Descriptor
tusb_desc_device_t const desc_device = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
	.idVendor = 0xFEED,
	.idProduct = 0x1187,
	.bcdDevice = 0x0100,
	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,
	.bNumConfigurations = 0x01};

// --- HID Report Descriptor ---
uint8_t const desc_hid_report[] = {
	// Report ID 1: Keyboard
	TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),

	// Report ID 2: Mouse
	TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),

	// Report ID 3: Configuration (Vendor Defined Feature Report)
	// Usage Page (Vendor Defined 0xFF00)
	0x06, 0x00, 0xFF,
	// Usage (Configuration)
	0x09, 0x01,
	// Collection (Application)
	0xA1, 0x01,
	// Report ID (3)
	0x85, REPORT_ID_CONFIG,
	// Usage (Data)
	0x09, 0x02,
	// Logical Min (0), Max (255)
	0x15, 0x00,
	0x26, 0xFF, 0x00,
	// Report Size (8 bits), Count (63 bytes) -> Total 64 bytes with ID
	0x75, 0x08,
	0x95, 0x3F,
	// Feature (Data, Var, Abs) - Host can Read/Write this
	0xB1, 0x02,
	// End Collection
	0xC0};

// Configuration Descriptor
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
uint8_t const desc_configuration[] = {
	TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
	TUD_HID_DESCRIPTOR(0, 4, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), 0x81, CFG_TUD_HID_EP_BUFSIZE, 10)};

// String Descriptors
char const *string_desc_arr[] = {
	(const char[]){0x09, 0x04}, // 0: English
	"Baseline Design",			// 1: Manufacturer
	"KNOB Macro Pad",			// 2: Product
	"123456",					// 3: Serial
};

// -- Callbacks --
uint8_t const *tud_descriptor_device_cb(void) { return (uint8_t const *)&desc_device; }
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) { return desc_configuration; }
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { return desc_hid_report; }
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	static uint16_t _desc_str[32];
	uint8_t chr_count;

	if (index == 0)
	{
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		chr_count = 1;
	}
	else
	{
		if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
			return NULL;
		const char *str = string_desc_arr[index];
		chr_count = strlen(str);
		for (uint8_t i = 0; i < chr_count; i++)
			_desc_str[1 + i] = str[i];
	}
	_desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
	return _desc_str;
}
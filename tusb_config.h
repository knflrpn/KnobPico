#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// Defined by the board in the Pico SDK, but we ensure defaults here
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT      0
#endif

#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

// --- Device Configuration ---
#define CFG_TUD_ENABLED         1
#define CFG_TUD_HID             1  // Enable HID Class
#define CFG_TUD_CDC             0  // Disable CDC (Serial) to keep it simple

// --- Buffer Configuration ---
#define CFG_TUD_HID_EP_BUFSIZE  64

#ifdef __cplusplus
}
#endif

#endif
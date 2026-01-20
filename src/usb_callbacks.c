#include "tusb.h"

#include <string.h>

#include "knobpico_state.h"
#include "knobpico_usb_config.h"
#include "usb_transport.h"

uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer,
                               uint16_t reqlen)
{
  (void)instance;

  if (report_id == REPORT_ID_CONFIG && report_type == HID_REPORT_TYPE_FEATURE)
  {
    KnobPicoState *st = knobpico_state();
    uint16_t n = sizeof(BkTransportPacket);
    if (n > reqlen)
      n = reqlen;
    memcpy(buffer, &st->last_response, n);
    return n;
  }

  return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer,
                           uint16_t bufsize)
{
  (void)instance;

  if (report_id == REPORT_ID_CONFIG && report_type == HID_REPORT_TYPE_FEATURE)
  {
    if (bufsize >= sizeof(BkTransportPacket))
    {
      KnobPicoState *st = knobpico_state();
      BkTransportPacket pkt;
      memcpy(&pkt, buffer, sizeof(pkt));

      usb_config_handle_command(&pkt);
      memcpy(&st->last_response, &pkt, sizeof(pkt));
    }
  }
}

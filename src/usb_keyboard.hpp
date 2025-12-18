#pragma once

#include <cstdint>
#include "tusb.h"
#include "class/hid/hid_device.h"

#ifdef __cplusplus
extern "C" {
#endif

void usb_device_task(void *params);

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);

#ifdef __cplusplus
}
#endif

void usb_device_init(void);
bool key_exists_in_report(uint8_t *key, int index);
bool send_usb(uint8_t *key);
bool clear_usb_report(void);
void send_string(const char* str);

extern uint8_t modifier;
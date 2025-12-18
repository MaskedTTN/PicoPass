#pragma once

#define CFG_TUSB_MCU              OPT_MCU_RP2040
#define CFG_TUSB_RHPORT0_MODE     OPT_MODE_DEVICE
#define CFG_TUSB_RHPORT0_SPEED    OPT_MODE_FULL_SPEED

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_FREERTOS
#endif

#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN        __attribute__ ((aligned(4)))

#define CFG_TUD_HID               1 

// Enable CDC for USB stdio
#define CFG_TUD_CDC               1
#define CFG_TUD_CDC_EP_BUFSIZE    64
#define CFG_TUD_CDC_RX_BUFSIZE    64
#define CFG_TUD_CDC_TX_BUFSIZE    64


// HID buffer size
#define CFG_TUD_HID_EP_BUFSIZE    64
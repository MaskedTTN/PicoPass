#include "pico/stdlib.h"
#include "tusb.h"
#include "class/hid/hid_device.h"
#include <FreeRTOS.h>
#include <task.h>
#include <cstring>

#include "bsp/board.h"
#include "usb_descriptors.h"
#include "keys.h"

// Report size defined by tinyUSB
#define KEY_REPORT_SIZE 6

uint8_t modifier;

bool key_exists_in_report(uint8_t *key, int index)
{
    bool ret = false;

    for(int i = index-1; i > 0; --i)
    {
        if (key[index] == key[i])
        {
            ret = true;
        }
    }

    return ret;
}

bool send_usb(uint8_t *key)
{
    uint8_t key_report[6] = {0};
    bool default_key = false;
    
    for (int i = 0; i < KEY_REPORT_SIZE; ++i)
    {
        switch (key[i])
        {
            case K_SHFT:
                modifier |= MOD_SHFT;
                break;
            case K_CTRL:
                modifier |= MOD_CTRL;
                break;
            case K_ALT:
                modifier |= MOD_ALT;
                break;
            case 0:
                break;
            default:
                default_key = true;
                key_report[i] = key[i];
                break;
        }
    }
    if (default_key == true) {
        tud_hid_keyboard_report(0, modifier, key_report);
        modifier = 0;
    }

    return true;
}

bool clear_usb_report(void)
{
    uint8_t key_report[6] = {0};
    return tud_hid_keyboard_report(0, 0, key_report);
}

// Simple function to send a string as keystrokes
void send_string(const char* str) {
    // Check if USB is ready first
    if (!tud_ready()) {
        printf("USB not ready, cannot send string\n");
        return;
    }
    
    for (size_t i = 0; i < strlen(str); i++) {
        uint8_t keycode = 0;
        uint8_t modifier = 0;
        
        // Convert character to HID keycode
        char ch = str[i];
        
        if (ch >= 'a' && ch <= 'z') {
            keycode = HID_KEY_A + (ch - 'a');
        }
        else if (ch >= 'A' && ch <= 'Z') {
            keycode = HID_KEY_A + (ch - 'A');
            modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        }
        else if (ch >= '1' && ch <= '9') {
            keycode = HID_KEY_1 + (ch - '1');
        }
        else if (ch == '0') {
            keycode = HID_KEY_0;
        }
        else if (ch == ' ') {
            keycode = HID_KEY_SPACE;
        }
        
        if (keycode != 0) {
            // Wait until ready with timeout
            int timeout = 100; // 500ms timeout
            while (!tud_hid_ready() && timeout-- > 0) {
                vTaskDelay(pdMS_TO_TICKS(5));
            }
            
            if (timeout <= 0) {
                printf("Timeout waiting for HID ready\n");
                return;
            }
            
            // Press key
            uint8_t keycodes[6] = {keycode, 0, 0, 0, 0, 0};
            tud_hid_keyboard_report(0, modifier, keycodes);
            vTaskDelay(pdMS_TO_TICKS(10));
            
            // Release key
            tud_hid_keyboard_report(0, 0, NULL);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

extern "C" void usb_device_task(void *param) {
    modifier = 0;
    board_init();
    tusb_init();
    
    // Wait for USB to be mounted/ready
    while (!tud_ready()) {
        tud_task();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    printf("USB device ready!\n");
    
    while (1) {
        tud_task(); // Process USB events
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


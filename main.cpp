/**
 * PicoPass - Physical Password Manager for Raspberry Pi Pico
 * Uses Pimoroni Display Pack and USB HID to type passwords
 * 
 * Note: This file uses C++ due to Pimoroni library requirements,
 * but the code style is C-like for compatibility.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "pimoroni_pico_display.hpp"

using namespace pimoroni;

// Button GPIO pins (adjust based on your Pimoroni display pack)
#define BUTTON_A_PIN 12
#define BUTTON_B_PIN 13
#define BUTTON_X_PIN 14
#define BUTTON_Y_PIN 15

// Password storage structure
typedef struct {
    char name[32];
    char password[128];
} password_entry_t;

// Password database
#define MAX_PASSWORDS 20
password_entry_t passwords[MAX_PASSWORDS];
int password_count = 0;

// Display instance
PicoDisplay display;

/**
 * Initialize default passwords
 */
void init_passwords(void) {
    password_count = 3;
    
    strcpy(passwords[0].name, "Gmail");
    strcpy(passwords[0].password, "example_password_123");
    
    strcpy(passwords[1].name, "GitHub");
    strcpy(passwords[1].password, "github_secure_pass");
    
    strcpy(passwords[2].name, "Bank");
    strcpy(passwords[2].password, "banking_password_456");
}

/**
 * Wait for USB HID to be ready
 */
void wait_for_usb_hid(void) {
    while (!tud_hid_ready()) {
        tud_task();
        sleep_ms(10);
    }
}

/**
 * Type a string using USB HID keyboard
 */
void type_string(const char *text) {
    wait_for_usb_hid();
    
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        uint8_t keycode = 0;
        uint8_t modifier = 0;
        
        // Handle uppercase letters
        if (c >= 'A' && c <= 'Z') {
            keycode = HID_KEY_A + (c - 'A');
            modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        }
        // Handle lowercase letters
        else if (c >= 'a' && c <= 'z') {
            keycode = HID_KEY_A + (c - 'a');
        }
        // Handle numbers
        else if (c >= '0' && c <= '9') {
            keycode = HID_KEY_0 + (c - '0');
        }
        // Handle space
        else if (c == ' ') {
            keycode = HID_KEY_SPACE;
        }
        // Handle special characters
        else {
            switch (c) {
                case '!': keycode = HID_KEY_1; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '@': keycode = HID_KEY_2; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '#': keycode = HID_KEY_3; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '$': keycode = HID_KEY_4; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '%': keycode = HID_KEY_5; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '^': keycode = HID_KEY_6; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '&': keycode = HID_KEY_7; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '*': keycode = HID_KEY_8; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '(': keycode = HID_KEY_9; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case ')': keycode = HID_KEY_0; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '_': keycode = HID_KEY_MINUS; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '+': keycode = HID_KEY_EQUAL; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '-': keycode = HID_KEY_MINUS; break;
                case '=': keycode = HID_KEY_EQUAL; break;
                case '[': keycode = HID_KEY_BRACKET_LEFT; break;
                case ']': keycode = HID_KEY_BRACKET_RIGHT; break;
                case '{': keycode = HID_KEY_BRACKET_LEFT; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '}': keycode = HID_KEY_BRACKET_RIGHT; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '\\': keycode = HID_KEY_BACKSLASH; break;
                case '|': keycode = HID_KEY_BACKSLASH; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case ';': keycode = HID_KEY_SEMICOLON; break;
                case ':': keycode = HID_KEY_SEMICOLON; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '\'': keycode = HID_KEY_APOSTROPHE; break;
                case '"': keycode = HID_KEY_APOSTROPHE; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case ',': keycode = HID_KEY_COMMA; break;
                case '<': keycode = HID_KEY_COMMA; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '.': keycode = HID_KEY_PERIOD; break;
                case '>': keycode = HID_KEY_PERIOD; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '/': keycode = HID_KEY_SLASH; break;
                case '?': keycode = HID_KEY_SLASH; modifier = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                case '\n': keycode = HID_KEY_ENTER; break;
                default: continue; // Skip unknown characters
            }
        }
        
        if (keycode != 0) {
            // Press key
            uint8_t keyreport[6] = {0};
            keyreport[2] = keycode;
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keyreport + 2);
            sleep_ms(10);
            
            // Release key
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            sleep_ms(10);
        }
        
        tud_task(); // Process USB events
    }
}

/**
 * Clear display
 */
void clear_display(void) {
    display.set_pen(0, 0, 0);
    display.clear();
    display.update();
}

/**
 * Draw text on display
 */
void draw_text(const char *text, int x, int y, int scale, uint8_t r, uint8_t g, uint8_t b) {
    display.set_pen(r, g, b);
    display.text(text, Point(x, y), scale);
}

/**
 * Show password menu
 */
void show_menu(int selected_index) {
    clear_display();
    
    // Title
    draw_text("PicoPass", 10, 10, 3, 0, 255, 0);
    
    // Menu items
    int start_idx = (selected_index > 2) ? selected_index - 2 : 0;
    int end_idx = (start_idx + 4 < password_count) ? start_idx + 4 : password_count;
    
    int y_offset = 50;
    for (int i = start_idx; i < end_idx; i++) {
        int idx = i - start_idx;
        bool is_selected = (i == selected_index);
        uint8_t r = is_selected ? 0 : 255;
        uint8_t g = is_selected ? 255 : 255;
        uint8_t b = is_selected ? 0 : 255;
        const char *prefix = is_selected ? "> " : "  ";
        
        char line[64];
        snprintf(line, sizeof(line), "%s%s", prefix, passwords[i].name);
        draw_text(line, 10, y_offset + (idx * 25), 2, r, g, b);
    }
    
    // Instructions
    draw_text("A: Select  B: Exit", 10, display.bounds.h - 30, 1, 128, 128, 128);
    
    display.update();
}

/**
 * Show typing indicator
 */
void show_typing_indicator(const char *service_name) {
    clear_display();
    draw_text("Typing...", 10, 10, 3, 0, 255, 0);
    draw_text(service_name, 10, 50, 2, 255, 255, 255);
    display.update();
}

/**
 * Main program
 */
int main(void) {
    // Initialize USB
    stdio_init_all();
    tusb_init();
    
    // Initialize display
    display.init();
    display.set_backlight(255);
    
    // Initialize buttons
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
    
    gpio_init(BUTTON_X_PIN);
    gpio_set_dir(BUTTON_X_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_X_PIN);
    
    gpio_init(BUTTON_Y_PIN);
    gpio_set_dir(BUTTON_Y_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_Y_PIN);
    
    // Initialize passwords
    init_passwords();
    
    // Wait for USB HID to be ready
    wait_for_usb_hid();
    
    int selected_index = 0;
    
    if (password_count == 0) {
        clear_display();
        draw_text("No passwords", 10, 10, 2, 255, 0, 0);
        draw_text("configured!", 10, 40, 2, 255, 0, 0);
        display.update();
        return 0;
    }
    
    show_menu(selected_index);
    
    // Main loop
    while (true) {
        tud_task(); // TinyUSB task handler
        
        // Handle button A (Select)
        if (!gpio_get(BUTTON_A_PIN)) {
            if (password_count > 0) {
                const char *service_name = passwords[selected_index].name;
                const char *password = passwords[selected_index].password;
                
                show_typing_indicator(service_name);
                sleep_ms(500);
                
                type_string(password);
                
                // Show confirmation
                clear_display();
                draw_text("Sent!", 10, 10, 3, 0, 255, 0);
                draw_text(service_name, 10, 50, 2, 255, 255, 255);
                display.update();
                sleep_ms(1000);
                
                show_menu(selected_index);
            }
            
            // Debounce
            while (!gpio_get(BUTTON_A_PIN)) {
                tud_task();
                sleep_ms(10);
            }
        }
        
        // Handle button X (Navigate up)
        if (!gpio_get(BUTTON_X_PIN)) {
            selected_index = (selected_index - 1 + password_count) % password_count;
            show_menu(selected_index);
            while (!gpio_get(BUTTON_X_PIN)) {
                tud_task();
                sleep_ms(10);
            }
        }
        
        // Handle button Y (Navigate down)
        if (!gpio_get(BUTTON_Y_PIN)) {
            selected_index = (selected_index + 1) % password_count;
            show_menu(selected_index);
            while (!gpio_get(BUTTON_Y_PIN)) {
                tud_task();
                sleep_ms(10);
            }
        }
        
        sleep_ms(50);
    }
    
    return 0;
}


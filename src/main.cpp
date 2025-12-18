#include <FreeRTOS.h>
#include <task.h>
#include <cstdio>
#include "pico/stdlib.h"
#include "menu.hpp"
#include "shell.hpp"
#include "credentials.hpp"
#include "usb_keyboard.hpp"

// Global
Menu main_menu;
static bool led_status_enabled = false;

TaskHandle_t menu_task_handle = NULL;

void usb_hid_selected() {
    printf("USB HID Mode selected!\n");
    send_string("Hello from PicoPass!\n");
    if (shell_is_developer_mode()) {
        printf("[DEV] Developer mode is active\n");
    }
}

void credentials_selected() {
    printf("Credentials selected!\n");
    main_menu.enter_credentials_mode();
}

void settings_selected() {
    printf("Settings selected!\n");
    printf("Current settings:\n");
    printf("  Developer Mode: %s\n", shell_is_developer_mode() ? "ON" : "OFF");
    printf("  Debug Logging: %s\n", shell_is_debug_logging() ? "ON" : "OFF");
}

void about_selected() {
    printf("About selected!\n");
    printf("PicoPass v1.0\n");
    printf("Built with FreeRTOS\n");
}

void led_status_selected() {
    printf("LED Status selected!\n");
    
    main_menu.show_option_popup("LED Status", "Enabled", "Disabled", [](int selection) {
        if (selection == 0) {
            led_status_enabled = true;
            printf("LED Status: ENABLED\n");
        } else {
            led_status_enabled = false;
            printf("LED Status: DISABLED\n");
        }
    });
}

// Button debouncing
static uint32_t last_button_time = 0;
#define DEBOUNCE_MS 150

// Menu task - handles display and input
extern "C" void menu_task(void *params) {
    // Initialize menu
    main_menu.init("PicoPass");
    main_menu.add_item("Credentials", credentials_selected);
    main_menu.add_item("Settings", settings_selected);
    main_menu.add_item("About", about_selected);
    
    while (1) {
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // If popup is active, handle popup input
        if (main_menu.is_popup_active()) {
            main_menu.handle_popup_input(current_time, last_button_time);
        }
        else if (main_menu.is_credentials_mode()) {
            main_menu.handle_credentials_input(current_time, last_button_time);
        }
        else {
            static bool led_item_added = false;
            if (shell_is_developer_mode() && !led_item_added) {
                main_menu.add_item("LED Status", led_status_selected);
                led_item_added = true;
                printf("[DEV] LED Status menu item added\n");
            }

            if (current_time - last_button_time > DEBOUNCE_MS) {
                if (menu_button_pressed(BUTTON_A)) {  // Up
                    main_menu.move_up();
                    last_button_time = current_time;
                }
                else if (menu_button_pressed(BUTTON_B)) {  // Down
                    main_menu.move_down();
                    last_button_time = current_time;
                }
                else if (menu_button_pressed(BUTTON_X)) {  // Select
                    main_menu.select();
                    last_button_time = current_time;
                }
            }
        }
        
        main_menu.render();
        
        // Delay for smooth scrolling
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

extern "C" void blink_task(void *params) {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    while (1) {
        if (led_status_enabled) {
            gpio_put(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_put(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
        } else {
            gpio_put(LED_PIN, 0);  // Keep LED off
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

int main() {
    stdio_init_all();
    
    printf("PicoPass Starting...\n");

    xTaskCreate(shell_task, "Shell", 2048, NULL, 1, NULL);
    xTaskCreate(menu_task, "Menu", 2048, NULL, 2, &menu_task_handle);
    xTaskCreate(blink_task, "Blink", 256, NULL, 1, NULL);
    xTaskCreate(usb_device_task, "USB", 1024, NULL, 2, NULL);

    vTaskStartScheduler();
    
    // Should never reach here
    while (1) {}
    return 0;
}
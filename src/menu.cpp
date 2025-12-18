#include "menu.hpp"
#include "credentials.hpp"
#include "usb_keyboard.hpp"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "../drivers/st7789/st7789.hpp"
#include "../lib/pico_graphics/pico_graphics.hpp"

using namespace pimoroni;

// Display setup for Pimoroni Display Pack
static ST7789 st7789(240, 135, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
static PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

Menu::Menu() : item_count(0), selected_index(0), scroll_offset(0), title(nullptr), 
               show_popup(false), popup_title(nullptr), popup_selection(0),
               in_credentials_mode(false) {
    popup_options[0] = nullptr;
    popup_options[1] = nullptr;
}

void Menu::init(const char* menu_title) {
    item_count = 0;
    selected_index = 0;
    scroll_offset = 0;
    title = menu_title;
    
    // Initialize display
    st7789.set_backlight(255);
    
    // Initialize buttons
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    
    gpio_init(BUTTON_X);
    gpio_set_dir(BUTTON_X, GPIO_IN);
    gpio_pull_up(BUTTON_X);
    
    gpio_init(BUTTON_Y);
    gpio_set_dir(BUTTON_Y, GPIO_IN);
    gpio_pull_up(BUTTON_Y);
}

void Menu::add_item(const char* text, std::function<void()> callback) {
    if (item_count < MAX_MENU_ITEMS) {
        items[item_count] = MenuItem(text, callback);
        item_count++;
    }
}

void Menu::remove_item(const char* text) {
    for (uint8_t i = 0; i < item_count; i++) {
        if (strcmp(items[i].text, text) == 0) {
            // Shift items down to fill the gap
            for (uint8_t j = i; j < item_count - 1; j++) {
                items[j] = items[j + 1];
            }
            item_count--;
            // Adjust selected index if needed
            if (selected_index >= item_count && item_count > 0) {
                selected_index = item_count - 1;
            }
            break;
        }
    }
}

void Menu::move_up() {
    if (selected_index > 0) {
        selected_index--;
        
        // Adjust scroll offset if needed
        if (selected_index < scroll_offset) {
            scroll_offset = selected_index;
        }
    }
}

void Menu::move_down() {
    if (selected_index < item_count - 1) {
        selected_index++;
        
        // Adjust scroll offset if needed
        if (selected_index >= scroll_offset + VISIBLE_ITEMS) {
            scroll_offset = selected_index - VISIBLE_ITEMS + 1;
        }
    }
}

void Menu::select() {
    if (!show_popup && items[selected_index].callback) {
        items[selected_index].callback();
    }
}

void Menu::show_option_popup(const char* title, const char* option1, const char* option2, std::function<void(int)> callback) {
    show_popup = true;
    popup_title = title;
    popup_options[0] = option1;
    popup_options[1] = option2;
    popup_selection = 0;
    popup_callback = callback;
}

void Menu::handle_popup_input(uint32_t current_time, uint32_t& last_button_time) {
    if (!show_popup) return;
    
    if (current_time - last_button_time > 150) {
        if (menu_button_pressed(BUTTON_A)) {  // Up
            popup_selection = (popup_selection == 0) ? 1 : 0;
            last_button_time = current_time;
        }
        else if (menu_button_pressed(BUTTON_B)) {  // Down
            popup_selection = (popup_selection == 0) ? 1 : 0;
            last_button_time = current_time;
        }
        else if (menu_button_pressed(BUTTON_X)) {  // Select
            if (popup_callback) {
                popup_callback(popup_selection);
            }
            show_popup = false;
            last_button_time = current_time;
        }
        else if (menu_button_pressed(BUTTON_Y)) {  // Cancel
            show_popup = false;
            last_button_time = current_time;
        }
    }
}

void Menu::render() {
    // If in credentials mode, use credentials rendering
    if (in_credentials_mode && !show_popup) {
        render_credentials();
        return;
    }
    
    // Clear screen to black (Flipper Zero style)
    graphics.set_pen(0, 0, 0);
    graphics.clear();
    
    // Draw top bar with title (orange/white style)
    graphics.set_pen(255, 140, 0);  // Orange
    graphics.rectangle(Rect(0, 0, 240, 20));
    graphics.set_pen(0, 0, 0);  // Black text
    graphics.text(title, Point(8, 4), 240, 2);
    
    // If popup is active, render it
    if (show_popup) {
        // Draw semi-transparent overlay (just black rectangle)
        graphics.set_pen(0, 0, 0);
        graphics.rectangle(Rect(20, 35, 200, 80));
        
        // Draw popup border
        graphics.set_pen(255, 140, 0);  // Orange border
        graphics.rectangle(Rect(20, 35, 200, 3));  // Top
        graphics.rectangle(Rect(20, 112, 200, 3)); // Bottom
        graphics.rectangle(Rect(20, 35, 3, 80));   // Left
        graphics.rectangle(Rect(217, 35, 3, 80));  // Right
        
        // Draw popup title
        graphics.set_pen(255, 255, 255);
        graphics.text(popup_title, Point(30, 45), 180, 2);
        
        // Draw options
        for (int i = 0; i < 2; i++) {
            int y_pos = 70 + (i * 20);
            
            if (i == popup_selection) {
                graphics.set_pen(255, 140, 0);  // Orange highlight
                graphics.rectangle(Rect(25, y_pos - 2, 190, 18));
                graphics.set_pen(0, 0, 0);  // Black text
                graphics.text(">", Point(30, y_pos), 180, 2);
            } else {
                graphics.set_pen(255, 255, 255);  // White text
            }
            
            graphics.text(popup_options[i], Point(45, y_pos), 180, 2);
        }
        
        st7789.update(&graphics);
        return;
    }
    
    // Calculate visible range
    uint8_t start_idx = scroll_offset;
    uint8_t end_idx = start_idx + VISIBLE_ITEMS;
    if (end_idx > item_count) {
        end_idx = item_count;
    }
    
    // Draw menu items (3 visible at a time)
    int item_height = 35;
    int start_y = 30;
    
    for (uint8_t i = start_idx; i < end_idx; i++) {
        int y_pos = start_y + ((i - start_idx) * item_height);
        
        // Highlight selected item (Flipper style)
        if (i == selected_index) {
            graphics.set_pen(255, 140, 0);  // Orange background
            graphics.rectangle(Rect(0, y_pos - 2, 215, item_height - 2));
            graphics.set_pen(0, 0, 0);  // Black text
        } else {
            graphics.set_pen(255, 255, 255);  // White text
        }
        
        // Draw arrow for selected item
        if (i == selected_index) {
            graphics.text(">", Point(8, y_pos + 8), 200, 2);
        }
        
        // Draw menu item text
        graphics.text(items[i].text, Point(25, y_pos + 8), 180, 2);
    }
    
    // Draw scroll indicator on the right side (Flipper style)
    if (item_count > VISIBLE_ITEMS) {
        int scroll_bar_x = 220;
        int scroll_bar_y = 25;
        int scroll_bar_height = 105;
        
        // Draw scroll bar background
        graphics.set_pen(50, 50, 50);  // Dark gray
        graphics.rectangle(Rect(scroll_bar_x, scroll_bar_y, 6, scroll_bar_height));
        
        // Calculate indicator position and size
        float indicator_height = (float)scroll_bar_height * ((float)VISIBLE_ITEMS / item_count);
        float indicator_pos = (float)scroll_bar_y + 
            ((float)scroll_bar_height * ((float)scroll_offset / (item_count - VISIBLE_ITEMS)));
        
        // Draw scroll indicator
        graphics.set_pen(255, 140, 0);  // Orange
        graphics.rectangle(Rect(scroll_bar_x, (int)indicator_pos, 6, (int)indicator_height));
    }
    
    // Update display
    st7789.update(&graphics);
}

// Helper function to check buttons (non-blocking)
bool menu_button_pressed(uint gpio) {
    return !gpio_get(gpio);  // Buttons are active low
}

void Menu::enter_credentials_mode() {
    in_credentials_mode = true;
    selected_index = 0;
    scroll_offset = 0;
}

void Menu::exit_credentials_mode() {
    in_credentials_mode = false;
}

void Menu::handle_credentials_input(uint32_t current_time, uint32_t& last_button_time) {
    if (current_time - last_button_time > 150) {
        if (menu_button_pressed(BUTTON_A)) {  // Up
            if (selected_index > 0) {
                selected_index--;
                if (selected_index < scroll_offset) {
                    scroll_offset = selected_index;
                }
            }
            last_button_time = current_time;
        }
        else if (menu_button_pressed(BUTTON_B)) {  // Down
            size_t cred_count = g_credentials.get_count();
            if (selected_index < cred_count - 1) {
                selected_index++;
                if (selected_index >= scroll_offset + VISIBLE_ITEMS) {
                    scroll_offset = selected_index - VISIBLE_ITEMS + 1;
                }
            }
            last_button_time = current_time;
        }
        else if (menu_button_pressed(BUTTON_X)) {  // Select credential
            const Credential* cred = g_credentials.get_credential(selected_index);
            if (cred) {
                // Show popup to choose username or password
                show_option_popup(cred->title.c_str(), "Username", "Password", 
                    [cred](int selection) {
                        if (selection == 0) {
                            printf("Typing username: %s\n", cred->username.c_str());
                            send_string(cred->username.c_str());
                        } else {
                            printf("Typing password: %s\n", cred->password.c_str());
                            send_string(cred->password.c_str());
                        }
                    });
            }
            last_button_time = current_time;
        }
        else if (menu_button_pressed(BUTTON_Y)) {  // Back to main menu
            exit_credentials_mode();
            last_button_time = current_time;
        }
    }
}

void Menu::render_credentials() {
    // Clear screen to black
    graphics.set_pen(0, 0, 0);
    graphics.clear();
    
    // Draw top bar with title
    graphics.set_pen(255, 140, 0);  // Orange
    graphics.rectangle(Rect(0, 0, 240, 20));
    graphics.set_pen(0, 0, 0);  // Black text
    graphics.text("Credentials", Point(8, 4), 240, 2);
    
    size_t cred_count = g_credentials.get_count();
    
    if (cred_count == 0) {
        graphics.set_pen(255, 255, 255);
        graphics.text("No credentials", Point(40, 60), 200, 2);
        st7789.update(&graphics);
        return;
    }
    
    // Calculate visible range
    uint8_t start_idx = scroll_offset;
    uint8_t end_idx = start_idx + VISIBLE_ITEMS;
    if (end_idx > cred_count) {
        end_idx = cred_count;
    }
    
    // Draw credential items
    int item_height = 35;
    int start_y = 30;
    
    for (uint8_t i = start_idx; i < end_idx; i++) {
        int y_pos = start_y + ((i - start_idx) * item_height);
        const Credential* cred = g_credentials.get_credential(i);
        
        if (!cred) continue;
        
        // Highlight selected item
        if (i == selected_index) {
            graphics.set_pen(255, 140, 0);  // Orange background
            graphics.rectangle(Rect(0, y_pos - 2, 215, item_height - 2));
            graphics.set_pen(0, 0, 0);  // Black text
        } else {
            graphics.set_pen(255, 255, 255);  // White text
        }
        
        // Draw arrow for selected item
        if (i == selected_index) {
            graphics.text(">", Point(8, y_pos + 8), 200, 2);
        }
        
        // Draw credential title
        graphics.text(cred->title.c_str(), Point(25, y_pos + 8), 180, 2);
    }
    
    // Draw scroll indicator if needed
    if (cred_count > VISIBLE_ITEMS) {
        int scroll_bar_x = 220;
        int scroll_bar_y = 25;
        int scroll_bar_height = 105;
        
        graphics.set_pen(50, 50, 50);
        graphics.rectangle(Rect(scroll_bar_x, scroll_bar_y, 6, scroll_bar_height));
        
        float indicator_height = (float)scroll_bar_height * ((float)VISIBLE_ITEMS / cred_count);
        float indicator_pos = (float)scroll_bar_y + 
            ((float)scroll_bar_height * ((float)scroll_offset / (cred_count - VISIBLE_ITEMS)));
        
        graphics.set_pen(255, 140, 0);
        graphics.rectangle(Rect(scroll_bar_x, (int)indicator_pos, 6, (int)indicator_height));
    }
    
    st7789.update(&graphics);
}
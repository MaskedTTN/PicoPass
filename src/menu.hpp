#pragma once
#ifndef MENU_HPP
#define MENU_HPP

#include <cstdint>
#include <functional>
#include "pico/types.h"

#define MAX_MENU_ITEMS 20
#define VISIBLE_ITEMS 3

class MenuItem {
public:
    const char* text;
    std::function<void()> callback;
    
    MenuItem() : text(nullptr), callback(nullptr) {}
    MenuItem(const char* t, std::function<void()> cb) : text(t), callback(cb) {}
};

class Menu {
private:
    MenuItem items[MAX_MENU_ITEMS];
    uint8_t item_count;
    uint8_t selected_index;
    uint8_t scroll_offset;
    const char* title;
    bool show_popup;
    const char* popup_title;
    const char* popup_options[2];
    uint8_t popup_selection;
    bool in_credentials_mode;

public:
    Menu();
    void init(const char* title);
    void add_item(const char* text, std::function<void()> callback);
    void remove_item(const char* text);
    void move_up();
    void move_down();
    void select();
    void render();
    void show_option_popup(const char* title, const char* option1, const char* option2, std::function<void(int)> callback);
    void handle_popup_input(uint32_t current_time, uint32_t& last_button_time);
    bool is_popup_active() const { return show_popup; }
    
    // Credential mode
    void enter_credentials_mode();
    void exit_credentials_mode();
    bool is_credentials_mode() const { return in_credentials_mode; }
    void handle_credentials_input(uint32_t current_time, uint32_t& last_button_time);
    void render_credentials();
    
    uint8_t get_selected_index() const { return selected_index; }
    uint8_t get_item_count() const { return item_count; }
    
private:
    std::function<void(int)> popup_callback;
};

// Button definitions for Pimoroni Display Pack
#define BUTTON_A 12
#define BUTTON_B 13
#define BUTTON_X 14
#define BUTTON_Y 15

bool menu_button_pressed(uint gpio);

#endif // MENU_HPP
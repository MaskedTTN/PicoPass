#pragma once
#ifndef BOOTSEL_SCREEN_HPP
#define BOOTSEL_SCREEN_HPP

#include <cstdint>

// Display a BOOTSEL screen before entering bootloader mode
void show_bootsel_screen();


extern const uint8_t bootsel_image[];
extern const uint32_t bootsel_image_size;

#endif // BOOTSEL_SCREEN_HPP
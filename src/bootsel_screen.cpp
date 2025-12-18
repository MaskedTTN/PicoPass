#include "bootsel_screen.hpp"
#include "hardware/spi.h"
#include "../drivers/st7789/st7789.hpp"
#include "../lib/pico_graphics/pico_graphics.hpp"
#include "pico/stdlib.h"

//#define USE_CUSTOM_BOOTSEL_IMAGE 1

#ifdef USE_CUSTOM_BOOTSEL_IMAGE
#include "bootsel_image.h"
#endif

using namespace pimoroni;

const uint8_t usb_icon_24x24[] = {
    0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b01000000, 0b00000000,
    0b00000000, 0b01000000, 0b00000000,
    0b00000000, 0b01000000, 0b00000000,
    0b00000001, 0b11110000, 0b00000000,
    0b00000000, 0b01000000, 0b00000000,
    0b00000000, 0b01000011, 0b11000000,
    0b00000000, 0b01000011, 0b11000000,
    0b00011111, 0b11111100, 0b00000000,
    0b00000000, 0b01000000, 0b00000000,
    0b00000000, 0b01000000, 0b00000000,
    0b00011100, 0b01000000, 0b00000000,
    0b00011100, 0b01000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000,
    0b00111110, 0b00000000, 0b00000000,
    0b01100011, 0b00000000, 0b00000000,
    0b01100011, 0b00000000, 0b00000000,
    0b01100011, 0b00000000, 0b00000000,
    0b00111110, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000,
};

void show_bootsel_screen() {
    // Create display and graphics instances
    ST7789 st7789(240, 135, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));
    PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);
    
#ifdef USE_CUSTOM_BOOTSEL_IMAGE
    // Display custom background image if available
    for (uint32_t y = 0; y < bootsel_bg_height && y < 135; y++) {
        for (uint32_t x = 0; x < bootsel_bg_width && x < 240; x++) {
            uint8_t pixel = bootsel_bg[y * bootsel_bg_width + x];
            uint8_t r = ((pixel >> 5) & 0x07) * 36;  // Scale 3-bit to 8-bit
            uint8_t g = ((pixel >> 2) & 0x07) * 36;  // Scale 3-bit to 8-bit
            uint8_t b = (pixel & 0x03) * 85;         // Scale 2-bit to 8-bit
            graphics.set_pen(r, g, b);
            graphics.pixel(Point(x, y));
        }
    }
    
    // Add semi-transparent overlay for text readability
    graphics.set_pen(0, 0, 0);  // Black overlay
    for (int y = 40; y < 100; y++) {
        for (int x = 20; x < 220; x++) {
            if ((x + y) % 2 == 0) {  // Checkerboard pattern for transparency effect
                graphics.pixel(Point(x, y));
            }
        }
    }
    
    // Draw text on overlay
    graphics.set_pen(255, 255, 255);
    graphics.text("BOOTSEL MODE", Point(50, 50), 240, 2);
    graphics.text("Connect USB", Point(65, 75), 240, 2);
    
#else
    // Default simple design
    // Clear screen with dark background
    graphics.set_pen(0, 0, 0);
    graphics.clear();
    
    // Draw colored header bar
    graphics.set_pen(100, 50, 200);  // Purple
    graphics.rectangle(Rect(0, 0, 240, 30));
    
    // Draw title
    graphics.set_pen(255, 255, 255);
    graphics.text("BOOTSEL MODE", Point(50, 8), 240, 2);
    
    // Draw USB icon (centered at 108, 50)
    int icon_x = 108;
    int icon_y = 50;
    graphics.set_pen(255, 255, 255);
    
    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 24; x++) {
            int byte_index = y * 3 + (x / 8);
            int bit_index = 7 - (x % 8);
            
            if (usb_icon_24x24[byte_index] & (1 << bit_index)) {
                graphics.pixel(Point(icon_x + x, icon_y + y));
            }
        }
    }
    
    // Draw instruction text
    graphics.set_pen(200, 200, 200);
    graphics.text("Entering bootloader...", Point(35, 90), 240, 1);
    graphics.text("Connect USB to upload", Point(35, 105), 240, 1);
    graphics.text("firmware (UF2 file)", Point(45, 120), 240, 1);
#endif
    
    // Update display
    st7789.update(&graphics);
    
    // Keep display on for 2 seconds so user can see it
    sleep_ms(2000);
}
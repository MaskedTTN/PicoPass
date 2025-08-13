#include <map>

#include "pico_display.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"
#include "drivers/button/button.cpp"
#include "picosha2.hpp"
#include "secure_store.hpp"

#include "bsp/board.h"
#include "tusb.h"
#include "class/hid/hid.h"
#include "class/hid/hid_device.h"

using namespace pimoroni;

// Display driver
ST7789 st7789(PicoDisplay::WIDTH, PicoDisplay::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));

// Graphics library - in RGB332 mode you get 256 colours and optional dithering for ~32K RAM.
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

// RGB LED
RGBLED led(PicoDisplay::LED_R, PicoDisplay::LED_G, PicoDisplay::LED_B);

// And each button
Button button_a(PicoDisplay::A);
Button button_b(PicoDisplay::B);
Button button_x(PicoDisplay::X);
Button button_y(PicoDisplay::Y);

const std::string stored_hash = "f77f0ece0aa17656f081c581c06d2b216f5207570c69494f5c879659f03739bc"; //"ABXY"
std::string code = "";

struct Login
{
    std::string username;
    std::string password;
};

std::vector<Login> logins;

// Standard HID keyboard report descriptor
uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()};

extern "C" uint8_t const *tud_descriptor_device_cb(void)
{
    static const tusb_desc_device_t desc = {
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0200,

        .bDeviceClass = TUSB_CLASS_MISC,
        .bDeviceSubClass = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor = 0xCafe,
        .idProduct = 0x4000,
        .bcdDevice = 0x0100,

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 1};

    return (uint8_t const *)&desc;
}

extern "C" const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for single config

    // total descriptor length
    static const uint8_t desc_configuration[] = {
        // Config descriptor
        9, TUSB_DESC_CONFIGURATION, // bLength, bDescriptorType
        34, 0,                      // wTotalLength (LSB, MSB)
        1,                          // bNumInterfaces
        1,                          // bConfigurationValue
        0,                          // iConfiguration
        0x80,                       // bmAttributes (Bus-powered)
        50,                         // bMaxPower (in 2mA units) = 100mA

        // Interface descriptor (HID keyboard)
        9, TUSB_DESC_INTERFACE,
        0, 0, 1, TUSB_CLASS_HID, HID_SUBCLASS_BOOT, HID_ITF_PROTOCOL_KEYBOARD, 0,

        // HID descriptor
        9, HID_DESC_TYPE_HID,
        0x11, 0x01, // HID Class Spec release (1.11)
        0, 1, HID_DESC_TYPE_REPORT, sizeof(desc_hid_report), 0,

        // Endpoint descriptor (interrupt IN)
        7, TUSB_DESC_ENDPOINT,
        0x81, TUSB_XFER_INTERRUPT,
        8, 0, 10};

    return desc_configuration;
}

extern "C" const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    static uint16_t desc_str[32];

    const char *string_desc[] = {
        (const char[]){0x09, 0x04}, // 0: Supported language = English (0x0409)
        "Masked Titan",             // 1: Manufacturer
        "PicoPass",                 // 2: Product
        "123456",                   // 3: Serial
    };

    uint8_t chr_count;
    if (index == 0)
    {
        desc_str[1] = 0x0409;
        chr_count = 1;
    }
    else
    {
        const char *str = string_desc[index];
        chr_count = strlen(str);
        for (uint8_t i = 0; i < chr_count; i++)
        {
            desc_str[1 + i] = str[i];
        }
    }

    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return desc_str;
}

extern "C" const uint8_t *tud_hid_descriptor_report_cb(uint8_t instance)
{
    static const uint8_t desc_hid_report[] = {
        TUD_HID_REPORT_DESC_KEYBOARD()};
    return desc_hid_report;
}

extern "C" uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                                          hid_report_type_t report_type,
                                          uint8_t *buffer, uint16_t reqlen)
{
    return 0; // No report to return
}

extern "C" void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                                      hid_report_type_t report_type,
                                      const uint8_t *buffer, uint16_t bufsize)
{
    // Ignored for now
}

void type_hi()
{
    if (!tud_hid_ready())
        return;
    // uint8_t empty[6] = {0,0,0,0,0,0};
    // uint8_t keys[6] = { HID_KEY_H, 0, 0, 0, 0, 0 };
    uint8_t keycode[6] = {0};
    keycode[0] = HID_KEY_A;
    tud_hid_keyboard_report(0, 0, keycode);
    sleep_ms(200);
    tud_task();
    tud_hid_keyboard_report(0, 0, NULL); // release
    sleep_ms(200);
    tud_task();
    keycode[0] = HID_KEY_B;
    // keys[0] = HID_KEY_I;
    tud_hid_keyboard_report(0, 0, keycode);
    sleep_ms(200);
    tud_task();
    tud_hid_keyboard_report(0, 0, NULL); // release
    sleep_ms(200);
    tud_task();
}

/* uint8_t char_to_hid(char c, uint8_t* modifier) {
    *modifier = 0;

    if (c >= 'a' && c <= 'z') return HID_KEY_A + (c - 'a');
    if (c >= 'A' && c <= 'Z') {
        *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        return HID_KEY_A + (c - 'A');
    }
    if (c >= '0' && c <= '9') return HID_KEY_0 + (c - '0');
    if (c == ' ') return HID_KEY_SPACE;
    if (c == '\n') return HID_KEY_ENTER;

    // Add more symbols as needed (e.g., punctuation, special chars)
    return 0;
} */

bool char_to_hid(char c, uint8_t &modifier, uint8_t &keycode)
{
    modifier = 0;

    if (c >= 'a' && c <= 'z')
    {
        keycode = HID_KEY_A + (c - 'a');
    }
    else if (c >= 'A' && c <= 'Z')
    {
        modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        keycode = HID_KEY_A + (c - 'A');
    }
    else if (c >= '1' && c <= '9')
    {
        keycode = HID_KEY_1 + (c - '1'); // No Shift!
    }
    else if (c == '0')
    {
        keycode = HID_KEY_0;
    }
    else if (c == '!')
    {
        modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        keycode = HID_KEY_1;
    }
    else if (c == '@')
    {
        modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        keycode = HID_KEY_2;
    }
    else if (c == '.')
    {
        keycode = HID_KEY_PERIOD;
    }
    else if (c == '\t')
    {
        keycode = HID_KEY_TAB;
    }
    else if (c == '\n' || c == '\r')
    {
        keycode = HID_KEY_ENTER;
    }
    else
    {
        return false; // unsupported char
    }

    return true;
}

void type_password(const Login &login)
{
    std::string password = login.password;
    graphics.text("User: " + login.username, Point(10, 80), 200);

    uint8_t keys[6] = {0};

    for (char c : password)
    {
        while (!tud_hid_ready())
        {
            sleep_ms(1);
        }

        uint8_t modifier = 0;
        uint8_t keycode = 0;

        if (char_to_hid(c, modifier, keycode))
        {
            // Press key
            keys[0] = keycode;
            tud_hid_keyboard_report(0, modifier, keys);
            tud_task();
            sleep_ms(100);
        }
        // Release key
        keys[0] = 0;
        tud_hid_keyboard_report(0, 0, keys);
        tud_task();
        sleep_ms(100);
    }

    // Final release (ensure nothing is left pressed)
    memset(keys, 0, sizeof(keys));
    tud_hid_keyboard_report(0, 0, keys);
    tud_task();
}

void draw_login(const Login &login)
{
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("Select Login:", Point(10, 20), 200);

    graphics.text("User: " + login.password, Point(10, 60), 200);

    graphics.text("A: Prev  X: Next", Point(10, 120), 200);

    st7789.update(&graphics);
}

void menu()
{
    board_init();
    tusb_init();
    int curr_login_index = 0;
    draw_login(logins[curr_login_index]);
    while (true)
    {
        tud_task();
        if (button_x.raw())
        {
            curr_login_index = (curr_login_index + 1) % logins.size();
            draw_login(logins[curr_login_index]);
            sleep_ms(300); // debounce
        }

        if (button_a.raw())
        {
            curr_login_index = (curr_login_index - 1) % logins.size();
            draw_login(logins[curr_login_index]);
            sleep_ms(300); // debounce
        }

        if (button_y.raw())
        {
            // type_hi();
            type_password(logins[curr_login_index]);
            sleep_ms(2000);
        }
    }
}

void draw_lockscreen_ui()
{
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("Enter 4-letter code:", Point(10, 20), 200);

    graphics.text(code, Point(10, 60), 200);

    st7789.update(&graphics);
}

void draw_correct()
{
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("pin verified", Point(10, 20), 200);

    st7789.update(&graphics);
}

void draw_incorrect()
{
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("pin incorrect", Point(10, 20), 200);

    st7789.update(&graphics);
}
int main()
{
    // set the backlight to a value between 0 and 255
    // the backlight is driven via PWM and is gamma corrected by our
    // library to give a gorgeous linear brightness range.
    st7789.set_backlight(100);

    while (true)
    {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if (code.size() < 4)
        {
            if (button_a.raw())
            {
                code += "A";
                sleep_ms(300); // debounce
            }
            else if (button_b.raw())
            {
                code += "B";
                sleep_ms(300);
            }
            else if (button_x.raw())
            {
                code += "X";
                sleep_ms(300);
            }
            else if (button_y.raw())
            {
                code += "Y";
                sleep_ms(300);
            }
            draw_lockscreen_ui();
        }
        else
        {
            // validate
            std::vector<unsigned char> hash(picosha2::k_digest_size);
            std::string input_hash = picosha2::hash256_hex_string(code);
            std::vector<Login> tmp;
            if (secure_store::load(code, tmp))
            {
                logins = tmp;
            }
            else
            {
                // No data in flash yet — seed with defaults and save
                logins = {
                    {"user1@example.com", "pass1234"},
                    {"alice@example.com", "alicepwd"}};
                secure_store::save(code, logins);
            }
            // Compare hashes
            if (input_hash == stored_hash)
            {
                draw_correct();
                sleep_ms(1000);
                menu();
            }
            else
            {
                draw_incorrect();
            }
        }
    }
}
#include "pico_display.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "rgbled.hpp"
#include "drivers/button/button.cpp"
#include "picosha2.hpp"

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

const std::string stored_hash = "f77f0ece0aa17656f081c581c06d2b216f5207570c69494f5c879659f03739bc";//"ABXY"
std::string code = "";

// Struct for login info
struct Login {
    std::string username;
    std::string password;
};

// Example logins
std::vector<Login> logins = {
    {"user1@example.com", "pass1234"},
    {"alice@example.com", "alicepwd"},
    {"bob@example.com", "b0bpwd!"}
};

void draw_login(const Login &login) {
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("Select Login:", Point(10, 20), 200);

    graphics.text("User: " + login.username, Point(10, 60), 200);

    graphics.text("A: Prev  X: Next", Point(10, 120), 200);

    st7789.update(&graphics);
}

void menu () {
    int curr_login_index = 0;
    draw_login(logins[curr_login_index]);
    while (true){
        if (button_x.raw()){
            curr_login_index = (curr_login_index + 1) % logins.size();
            draw_login(logins[curr_login_index]);
            sleep_ms(300); //debounce
        }

        if (button_a.raw()){
            curr_login_index = (curr_login_index - 1) % logins.size();
            draw_login(logins[curr_login_index]);
            sleep_ms(300); //debounce
        }
    }
}



void draw_lockscreen_ui() {
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("Enter 4-letter code:", Point(10, 20), 200);

    graphics.text(code, Point(10, 60), 200);

    st7789.update(&graphics);
}

void draw_correct() {
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("pin verified", Point(10, 20), 200);

    st7789.update(&graphics);
}

void draw_incorrect() {
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    graphics.set_pen(255, 255, 255);
    graphics.text("pin incorrect", Point(10, 20), 200);

    st7789.update(&graphics);
}
int main() {

    // set the backlight to a value between 0 and 255
    // the backlight is driven via PWM and is gamma corrected by our
    // library to give a gorgeous linear brightness range.
    st7789.set_backlight(100);
    
    
    while(true) {
        // detect if the A button is pressed (could be A, B, X, or Y)
        if(code.size() < 4) {
            if(button_a.raw()) {
                code += "A";
                sleep_ms(300); // debounce
            } else if(button_b.raw()) {
                code += "B";
                sleep_ms(300);
            } else if(button_x.raw()) {
                code += "X";
                sleep_ms(300);
            } else if(button_y.raw()) {
                code += "Y";
                sleep_ms(300);
            }
            draw_lockscreen_ui();
        } else {
            //validate 
            std::vector<unsigned char> hash(picosha2::k_digest_size);
            std::string input_hash = picosha2::hash256_hex_string(code);

            // Compare hashes
            if(input_hash == stored_hash) {
                draw_correct();
                sleep_ms(1000);
                menu();
            } else {
                draw_incorrect();
            }
        }
    }   
}
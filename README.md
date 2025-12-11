# PicoPass - Physical Password Manager

A physical password manager for Raspberry Pi Pico with Pimoroni Display Pack. Uses USB HID to type passwords as keyboard input.

## Features

- Navigate passwords using display buttons
- USB HID keyboard functionality to type passwords
- Simple menu interface on Pimoroni display
- Easy to add/modify passwords in code

## Hardware Requirements

- Raspberry Pi Pico (or Pico W)
- Pimoroni Pico Display Pack 2.0 (or compatible display)
- USB cable for power and data

## Software Requirements

- Raspberry Pi Pico SDK
- CMake (3.13 or higher)
- GCC ARM cross-compiler
- Pimoroni Pico Display library

## Setup Instructions

### 1. Install Pico SDK

```bash
# Clone the Pico SDK
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init

# Set environment variable
export PICO_SDK_PATH=/path/to/pico-sdk
```

### 2. Install Pimoroni Libraries

```bash
# Clone Pimoroni Pico C++ libraries
git clone https://github.com/pimoroni/pimoroni-pico.git
cd pimoroni-pico
git submodule update --init
```

### 3. Configure CMake

You may need to update `CMakeLists.txt` to point to the Pimoroni library location, or add it as a subdirectory:

```cmake
# Add this to CMakeLists.txt if needed
add_subdirectory(/path/to/pimoroni-pico pimoroni-pico)
```

### 4. Build the Project

```bash
mkdir build
cd build
cmake ..
make
```

### 5. Flash to Pico

1. Hold the BOOTSEL button on your Pico
2. Connect it to your computer via USB
3. Release BOOTSEL button
4. Copy `PicoPass.uf2` to the mounted drive

## Usage

1. Connect the Pico to your computer via USB
2. Wait for USB enumeration (may take a few seconds)
3. Use buttons to navigate:
   - **Button X**: Navigate up
   - **Button Y**: Navigate down
   - **Button A**: Select and type password
   - **Button B**: Exit (currently not implemented)
4. The selected password will be typed as keyboard input

## Customizing Passwords

Edit the `init_passwords()` function in `main.cpp` to add or modify passwords:

```c
void init_passwords(void) {
    password_count = 3;
    
    strcpy(passwords[0].name, "Gmail");
    strcpy(passwords[0].password, "your_password_here");
    
    // Add more passwords...
}
```

## Button Pin Configuration

The default button pins are:
- Button A: GPIO 12
- Button B: GPIO 13
- Button X: GPIO 14
- Button Y: GPIO 15

Adjust these in `main.cpp` if your display pack uses different pins.

## Security Note

⚠️ **WARNING**: Passwords are stored in plain text in the source code. This is a convenience tool, not a secure password manager. Do not use for sensitive passwords without additional encryption.

## Troubleshooting

- **USB not working**: Make sure TinyUSB is properly configured and the device is enumerated
- **Display not showing**: Check that the Pimoroni display library is properly linked
- **Buttons not responding**: Verify GPIO pin numbers match your hardware
- **Passwords not typing**: Ensure USB HID is working and the host computer recognizes the device as a keyboard

## License

This project is provided as-is for educational and personal use.


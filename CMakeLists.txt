cmake_minimum_required(VERSION 3.13)

# Include pico_sdk_import.cmake from the Pico SDK
include(pico_sdk_import.cmake)

project(PicoPass)

# Initialize the Pico SDK
pico_sdk_init()

# Add executable
add_executable(${PROJECT_NAME}
    main.cpp
    usb_descriptors.c
)

# Add TinyUSB library
pico_add_extra_outputs(${PROJECT_NAME})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_time
    hardware_gpio
    pimoroni_pico_display
    tinyusb_device
)

# Enable USB device support
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# TinyUSB configuration
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUD_HID=1
    CFG_TUD_CDC=1
    CFG_TUD_MSC=0
    CFG_TUD_MIDI=0
    CFG_TUD_VENDOR=0
)


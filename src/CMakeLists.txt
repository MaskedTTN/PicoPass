add_executable(${ProjectName}
    main.cpp
    menu.cpp
    shell.cpp
    usb_keyboard.cpp
    usb_descriptors.cpp
    flash_store.cpp
    credentials.cpp
    bootsel_screen.cpp
)

target_include_directories(${ProjectName} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${ProjectName} 
    pico_stdlib 
    FreeRTOS-Kernel-Heap4 
    hardware_spi
    hardware_pwm
    hardware_dma
    rgbled
    pico_display
    pico_graphics
    st7789
    tinyusb_device
    tinyusb_board
    )

pico_add_extra_outputs(${ProjectName})
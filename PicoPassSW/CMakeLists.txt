# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoPassSW C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

pico_find_compiler(PICO_COMPILER_LD ${PICO_GCC_TRIPLE}-ld)

function(add_resource target file)
    get_filename_component(NAME ${ARGV1} NAME_WE)
    set(FILENAME ${ARGV1})

    add_custom_command(
        OUTPUT ${NAME}.o

        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}
                ${CMAKE_CURRENT_BINARY_DIR}
        
        COMMAND ${PICO_COMPILER_LD} -r -b binary -o ${NAME}.o ${FILENAME}
        DEPENDS ${FILENAME}
    )
    target_sources(${ARGV0} PRIVATE ${NAME}.o)
endfunction(add_resource)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_subdirectory(common)
add_subdirectory(drivers)
add_subdirectory(libraries)

# ---- Main target ----
add_executable(
    PicoPassSW
    PicoPassSW.cpp
    secure_store.cpp     # <-- compile into the app
    flash_store.cpp      # <-- compile into the app
)

pico_enable_stdio_usb(PicoPassSW 1)
pico_enable_stdio_uart(PicoPassSW 0)

target_include_directories(PicoPassSW PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(PicoPassSW
    pico_stdlib
    hardware_spi
    hardware_pwm
    hardware_dma
    hardware_flash       # <-- needed by flash_store.cpp
    hardware_sync
    rgbled
    pico_display
    pico_graphics
    st7789
    tinyusb_device
    tinyusb_board
)

# Flash geometry / reserved region for encrypted blob
target_compile_definitions(PicoPassSW PRIVATE
    PICO_FLASH_SIZE_BYTES=2097152
    FLASH_STORE_SIZE=65536
)

# create map/bin/hex file etc.
pico_add_extra_outputs(PicoPassSW)

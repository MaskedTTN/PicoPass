# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
# Add executable. Default name is the project name, version 0.1

#add_executable(PicoPassSW PicoPassSW.cpp )

#pico_set_program_name(PicoPassSW "PicoPassSW")
#pico_set_program_version(PicoPassSW "0.1")

# Modify the below lines to enable/disable output over UART/USB
#pico_enable_stdio_uart(PicoPassSW 0)
#pico_enable_stdio_usb(PicoPassSW 0)

# Add the standard library to the build
#target_link_libraries(PicoPassSW
##        pico_stdlib)

# Add the standard include files to the build
#target_include_directories(PicoPassSW PRIVATE
#        ${CMAKE_CURRENT_LIST_DIR}
#)

#pico_add_extra_outputs(PicoPassSW)

add_subdirectory(common)
add_subdirectory(drivers)
add_subdirectory(libraries)

add_executable(
    pico_display_demo
    PicoPassSW.cpp
    image_data.cpp
)

target_link_libraries(pico_display_demo pico_stdlib hardware_spi hardware_pwm hardware_dma rgbled pico_display pico_graphics st7789)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_display_demo)
#pragma once
#ifndef SHELL_HPP
#define SHELL_HPP

#include <FreeRTOS.h>
#include <task.h>

// Shell task function
extern "C" void shell_task(void *params);

// Getter functions for settings
bool shell_is_developer_mode();
bool shell_is_debug_logging();
bool shell_is_usb_enabled();

#endif // SHELL_HPP
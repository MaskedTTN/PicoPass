#include "shell.hpp"
#include "credentials.hpp"
#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "menu.hpp"
#include "bootsel_screen.hpp"

extern Menu main_menu;
extern void usb_hid_selected();
extern TaskHandle_t menu_task_handle;


// Global settings
static bool developer_mode = false;
static bool debug_logging = false;
static bool usb_hid_enabled = true;

// Command buffer
#define CMD_BUFFER_SIZE 128
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint8_t cmd_index = 0;

// Helper function to check for --json flag
bool has_json_flag(const char* arg) {
    return arg && strstr(arg, "--json") != nullptr;
}

// Helper function to escape JSON strings
void print_json_escaped(const char* str) {
    while (*str) {
        switch (*str) {
            case '"':  printf("\\\""); break;
            case '\\': printf("\\\\"); break;
            case '\b': printf("\\b"); break;
            case '\f': printf("\\f"); break;
            case '\n': printf("\\n"); break;
            case '\r': printf("\\r"); break;
            case '\t': printf("\\t"); break;
            default:
                if (*str < 32) {
                    printf("\\u%04x", *str);
                } else {
                    putchar(*str);
                }
                break;
        }
        str++;
    }
}

// Command handlers
void cmd_help() {
    printf("\n=== PicoPass Shell ===\n");
    printf("Available commands:\n");
    printf("  help           - Show this help\n");
    printf("  status [--json] - Show current settings\n");
    printf("  devmode on/off - Enable/disable developer mode\n");
    printf("  debug on/off   - Enable/disable debug logging\n");
    printf("  usb on/off     - Enable/disable USB HID\n");
    printf("  cred list [--json] - List all credentials\n");
    printf("  cred add [--json '{...}'] - Add a new credential\n");
    printf("  cred del <id>  - Delete credential by ID\n");
    printf("  cred clear     - Clear all credentials\n");
    printf("  cred save      - Save credentials to flash\n");
    printf("  cred load      - Load credentials from flash\n");
    printf("  clear          - Clear screen\n");
    printf("  reboot         - Reboot device\n");
    printf("\nJSON Examples:\n");
    printf("  status --json\n");
    printf("  cred list --json\n");
    printf("  cred add --json '{\"title\":\"Gmail\",\"user\":\"me@example.com\",\"pass\":\"secret\"}'\n");
    printf("====================\n\n");
}

void cmd_status(const char* arg) {
    if (has_json_flag(arg)) {
        // JSON output
        printf("{\"developer_mode\":%s,\"debug_logging\":%s,\"usb_hid\":%s}\n",
               developer_mode ? "true" : "false",
               debug_logging ? "true" : "false",
               usb_hid_enabled ? "true" : "false");
    } else {
        // Human-readable output
        printf("\n=== System Status ===\n");
        printf("Developer Mode: %s\n", developer_mode ? "ON" : "OFF");
        printf("Debug Logging:  %s\n", debug_logging ? "ON" : "OFF");
        printf("USB HID:        %s\n", usb_hid_enabled ? "ON" : "OFF");
        printf("====================\n\n");
    }
}

void cmd_devmode(const char* arg) {
    if (strcmp(arg, "on") == 0) {
        developer_mode = true;
        main_menu.add_item("USB HID", usb_hid_selected);
        printf("Developer mode ENABLED\n");
    } else if (strcmp(arg, "off") == 0) {
        developer_mode = false;
        main_menu.remove_item("USB HID");
        printf("Developer mode DISABLED\n");
    } else {
        printf("Usage: devmode on/off\n");
    }
}

void cmd_debug(const char* arg) {
    if (strcmp(arg, "on") == 0) {
        debug_logging = true;
        printf("Debug logging ENABLED\n");
    } else if (strcmp(arg, "off") == 0) {
        debug_logging = false;
        printf("Debug logging DISABLED\n");
    } else {
        printf("Usage: debug on/off\n");
    }
}

void cmd_usb(const char* arg) {
    if (strcmp(arg, "on") == 0) {
        usb_hid_enabled = true;
        printf("USB HID ENABLED\n");
    } else if (strcmp(arg, "off") == 0) {
        usb_hid_enabled = false;
        printf("USB HID DISABLED\n");
    } else {
        printf("Usage: usb on/off\n");
    }
}

void cmd_clear() {
    printf("\033[2J\033[H");  // ANSI escape codes to clear screen
}

void cmd_reboot(const char* arg) {
    if (arg && strcmp(arg, "bootsel") == 0) {
        printf("Rebooting to BOOTSEL mode...\n");
        vTaskSuspend(menu_task_handle);
        show_bootsel_screen();
        
        // Enter BOOTSEL mode
        reset_usb_boot(0, 0);
    } else {
        printf("Rebooting...\n");
        sleep_ms(500);
        watchdog_reboot(0, 0, 0);
    }
}

// Simple JSON parser for credential add
bool parse_json_credential(const char* json_str, char* title, char* username, char* password) {
    // Very basic JSON parser - looks for "title", "user", "pass" fields
    const char* pos = json_str;
    
    title[0] = username[0] = password[0] = '\0';
    
    // Find title
    const char* title_key = strstr(pos, "\"title\"");
    if (title_key) {
        const char* title_val = strchr(title_key + 7, '"');
        if (title_val) {
            title_val++;  // Skip opening quote
            const char* title_end = strchr(title_val, '"');
            if (title_end) {
                size_t len = title_end - title_val;
                if (len > 63) len = 63;
                strncpy(title, title_val, len);
                title[len] = '\0';
            }
        }
    }
    
    // Find user
    const char* user_key = strstr(pos, "\"user\"");
    if (user_key) {
        const char* user_val = strchr(user_key + 6, '"');
        if (user_val) {
            user_val++;
            const char* user_end = strchr(user_val, '"');
            if (user_end) {
                size_t len = user_end - user_val;
                if (len > 63) len = 63;
                strncpy(username, user_val, len);
                username[len] = '\0';
            }
        }
    }
    
    // Find pass
    const char* pass_key = strstr(pos, "\"pass\"");
    if (pass_key) {
        const char* pass_val = strchr(pass_key + 6, '"');
        if (pass_val) {
            pass_val++;
            const char* pass_end = strchr(pass_val, '"');
            if (pass_end) {
                size_t len = pass_end - pass_val;
                if (len > 63) len = 63;
                strncpy(password, pass_val, len);
                password[len] = '\0';
            }
        }
    }
    
    return strlen(title) > 0;
}

// Credential management commands
void cmd_cred(const char* arg) {
    if (!arg || strlen(arg) == 0) {
        printf("Usage: cred [list|add|del|clear|save|load]\n");
        return;
    }
    
    // Parse subcommand
    char subcmd[32] = {0};
    const char* rest = arg;
    
    // Extract first word
    int i = 0;
    while (*rest && *rest != ' ' && i < 31) {
        subcmd[i++] = *rest++;
    }
    subcmd[i] = '\0';
    
    // Skip spaces
    while (*rest == ' ') rest++;
    
    if (strcmp(subcmd, "list") == 0) {
        size_t count = g_credentials.get_count();
        
        if (has_json_flag(rest)) {
            // JSON output
            printf("[");
            for (size_t i = 0; i < count; i++) {
                const Credential* cred = g_credentials.get_credential(i);
                if (cred) {
                    if (i > 0) printf(",");
                    printf("{\"id\":%zu,\"title\":\"", i);
                    print_json_escaped(cred->title.c_str());
                    printf("\",\"username\":\"");
                    print_json_escaped(cred->username.c_str());
                    printf("\",\"password\":\"");
                    print_json_escaped(cred->password.c_str());
                    printf("\"}");
                }
            }
            printf("]\n");
        } else {
            // Human-readable output
            printf("\n=== Credentials (%zu) ===\n", count);
            
            if (count == 0) {
                printf("No credentials stored.\n");
            } else {
                for (size_t i = 0; i < count; i++) {
                    const Credential* cred = g_credentials.get_credential(i);
                    if (cred) {
                        printf("[%zu] %s\n", i, cred->title.c_str());
                        printf("    Username: %s\n", cred->username.c_str());
                        printf("    Password: %s\n", cred->password.c_str());
                    }
                }
            }
            printf("===================\n\n");
        }
    }
    else if (strcmp(subcmd, "add") == 0) {
        if (has_json_flag(rest)) {
            // JSON mode
            const char* json_start = strchr(rest, '{');
            if (!json_start) {
                printf("{\"error\":\"Invalid JSON format\"}\n");
                return;
            }
            
            char title[64] = {0};
            char username[64] = {0};
            char password[64] = {0};
            
            if (parse_json_credential(json_start, title, username, password)) {
                g_credentials.add_credential(title, username, password);
                
                if (g_credentials.save_to_flash()) {
                    printf("{\"success\":true,\"title\":\"");
                    print_json_escaped(title);
                    printf("\"}\n");
                } else {
                    printf("{\"success\":false,\"error\":\"Failed to save to flash\"}\n");
                }
            } else {
                printf("{\"error\":\"Failed to parse JSON\"}\n");
            }
        } else {
            // Interactive mode
            printf("\n=== Add New Credential ===\n");
            
            char title[64] = {0};
            char username[64] = {0};
            char password[64] = {0};
            
            printf("Title: ");
            int idx = 0;
            while (idx < 63) {
                int c = getchar();
                if (c == '\r' || c == '\n') break;
                if (c == 127 || c == 8) {
                    if (idx > 0) {
                        idx--;
                        printf("\b \b");
                    }
                } else if (c >= 32 && c < 127) {
                    title[idx++] = c;
                    putchar(c);
                }
            }
            title[idx] = '\0';
            printf("\n");
            
            if (strlen(title) == 0) {
                printf("Cancelled: Title cannot be empty\n");
                return;
            }
            
            printf("Username: ");
            idx = 0;
            while (idx < 63) {
                int c = getchar();
                if (c == '\r' || c == '\n') break;
                if (c == 127 || c == 8) {
                    if (idx > 0) {
                        idx--;
                        printf("\b \b");
                    }
                } else if (c >= 32 && c < 127) {
                    username[idx++] = c;
                    putchar(c);
                }
            }
            username[idx] = '\0';
            printf("\n");
            
            printf("Password: ");
            idx = 0;
            while (idx < 63) {
                int c = getchar();
                if (c == '\r' || c == '\n') break;
                if (c == 127 || c == 8) {
                    if (idx > 0) {
                        idx--;
                        printf("\b \b");
                    }
                } else if (c >= 32 && c < 127) {
                    password[idx++] = c;
                    putchar(c);
                }
            }
            password[idx] = '\0';
            printf("\n");
            
            g_credentials.add_credential(title, username, password);
            printf("Credential '%s' added successfully!\n", title);
            printf("Saving credentials to flash...\n");
            if (g_credentials.save_to_flash()) {
                printf("Credentials saved successfully!\n");
            } else {
                printf("Error: Failed to save credentials\n");
            }
        }
    }
    else if (strcmp(subcmd, "del") == 0) {
        if (strlen(rest) == 0) {
            printf("Usage: cred del <id>\n");
            return;
        }
        
        int id = atoi(rest);
        if (id < 0 || id >= (int)g_credentials.get_count()) {
            printf("Error: Invalid credential ID\n");
            return;
        }
        
        const Credential* cred = g_credentials.get_credential(id);
        if (cred) {
            printf("Deleting credential: %s\n", cred->title.c_str());
            g_credentials.remove_credential(id);
            printf("Credential deleted.\n");
        }
        printf("Saving credentials to flash...\n");
        if (g_credentials.save_to_flash()) {
            printf("Credentials saved successfully!\n");
        } else {
            printf("Error: Failed to save credentials\n");
        }
    }
    else if (strcmp(subcmd, "clear") == 0) {
        printf("Are you sure you want to delete ALL credentials? (y/N): ");
        int c = getchar();
        printf("%c\n", c);
        
        if (c == 'y' || c == 'Y') {
            g_credentials.clear_all();
            printf("All credentials cleared.\n");
        } else {
            printf("Cancelled.\n");
        }
    }
    else if (strcmp(subcmd, "save") == 0) {
        printf("Saving credentials to flash...\n");
        if (g_credentials.save_to_flash()) {
            printf("Credentials saved successfully!\n");
        } else {
            printf("Error: Failed to save credentials\n");
        }
    }
    else if (strcmp(subcmd, "load") == 0) {
        printf("Loading credentials from flash...\n");
        if (g_credentials.load_from_flash()) {
            printf("Credentials loaded successfully! (%zu items)\n", g_credentials.get_count());
        } else {
            printf("Error: Failed to load credentials (or no credentials found)\n");
        }
    }
    else {
        printf("Unknown subcommand: %s\n", subcmd);
        printf("Usage: cred [list|add|del|clear|save|load]\n");
    }
}

// Parse and execute command
void shell_execute_command(char* cmd) {
    // Trim leading/trailing whitespace
    while (*cmd == ' ') cmd++;
    
    if (strlen(cmd) == 0) return;
    
    // Parse command and argument
    char* space = strchr(cmd, ' ');
    char* arg = nullptr;
    
    if (space) {
        *space = '\0';
        arg = space + 1;
        while (*arg == ' ') arg++;  // Skip leading spaces in arg
    }
    
    // Execute commands
    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    }
    else if (strcmp(cmd, "status") == 0) {
        cmd_status(arg);
    }
    else if (strcmp(cmd, "devmode") == 0) {
        if (arg) cmd_devmode(arg);
        else printf("Usage: devmode on/off\n");
    }
    else if (strcmp(cmd, "debug") == 0) {
        if (arg) cmd_debug(arg);
        else printf("Usage: debug on/off\n");
    }
    else if (strcmp(cmd, "usb") == 0) {
        if (arg) cmd_usb(arg);
        else printf("Usage: usb on/off\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        cmd_clear();
    }
    else if (strcmp(cmd, "reboot") == 0) {
        cmd_reboot(arg);
    }
    else if (strcmp(cmd, "cred") == 0) {
        cmd_cred(arg);
    }
    else {
        printf("Unknown command: %s\n", cmd);
        printf("Type 'help' for available commands\n");
    }
}

// Shell task
extern "C" void shell_task(void *params) {
    printf("\n\n");
    printf("╔═══════════════════════════════════════╗\n");
    printf("║     PicoPass Interactive Shell       ║\n");
    printf("║     Type 'help' for commands         ║\n");
    printf("╚═══════════════════════════════════════╝\n");
    printf("\n> ");
    
    while (1) {
        int c = getchar_timeout_us(0);  // Non-blocking
        
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                // Execute command
                printf("\n");
                cmd_buffer[cmd_index] = '\0';
                shell_execute_command(cmd_buffer);
                cmd_index = 0;
                printf("> ");
            }
            else if (c == 127 || c == 8) {  // Backspace
                if (cmd_index > 0) {
                    cmd_index--;
                    printf("\b \b");  // Erase character
                }
            }
            else if (c >= 32 && c < 127 && cmd_index < CMD_BUFFER_SIZE - 1) {
                // Printable character
                cmd_buffer[cmd_index++] = (char)c;
                putchar(c);  // Echo character
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  // Small delay
    }
}

// Getters for settings (so other modules can check them)
bool shell_is_developer_mode() {
    return developer_mode;
}

bool shell_is_debug_logging() {
    return debug_logging;
}

bool shell_is_usb_enabled() {
    return usb_hid_enabled;
}
#pragma once
#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

#include <vector>
#include <string>
#include <cstdint>

struct Credential {
    std::string title;
    std::string username;
    std::string password;
    
    Credential() = default;
    Credential(const char* t, const char* u, const char* p) 
        : title(t), username(u), password(p) {}
};

class CredentialManager {
private:
    std::vector<Credential> credentials;
    
public:
    CredentialManager();
    
    // Add/remove credentials
    void add_credential(const char* title, const char* username, const char* password);
    void remove_credential(size_t index);
    
    // Get credentials
    size_t get_count() const { return credentials.size(); }
    const Credential* get_credential(size_t index) const;
    
    // Load/save from flash
    bool load_from_flash();
    bool save_to_flash();
    
    // Clear all
    void clear_all();
};

// Global credential manager
extern CredentialManager g_credentials;

#endif // CREDENTIALS_HPP
#include "credentials.hpp"
#include "flash_store.hpp"
#include <cstring>
#include <cstdio>

CredentialManager g_credentials;

CredentialManager::CredentialManager() {
    // Try to load credentials from flash on startup
    if (!load_from_flash()) {
        // test credentials if none found
        printf("No credentials found in flash, adding defaults...\n");
        add_credential("GitHub", "user@example.com", "github_pass123");
        add_credential("Gmail", "myemail@gmail.com", "secure_password");
        add_credential("Work VPN", "john.doe", "vpn_secret");
    }
}

void CredentialManager::add_credential(const char* title, const char* username, const char* password) {
    credentials.push_back(Credential(title, username, password));
}

void CredentialManager::remove_credential(size_t index) {
    if (index < credentials.size()) {
        credentials.erase(credentials.begin() + index);
    }
}

const Credential* CredentialManager::get_credential(size_t index) const {
    if (index < credentials.size()) {
        return &credentials[index];
    }
    return nullptr;
}

void CredentialManager::clear_all() {
    credentials.clear();
}

bool CredentialManager::load_from_flash() {
    std::vector<uint8_t> blob;
    if (!flash_store::read_blob(blob)) {
        printf("No credentials found in flash\n");
        return false;
    }
    
    credentials.clear();
    
    // Simple serialization format:
    // [count:4][title_len:2][title][user_len:2][user][pass_len:2][pass]...
    size_t offset = 0;
    if (blob.size() < 4) return false;
    
    uint32_t count;
    std::memcpy(&count, blob.data(), 4);
    offset += 4;
    
    for (uint32_t i = 0; i < count && offset < blob.size(); i++) {
        // Read title
        if (offset + 2 > blob.size()) break;
        uint16_t title_len;
        std::memcpy(&title_len, blob.data() + offset, 2);
        offset += 2;
        
        if (offset + title_len > blob.size()) break;
        std::string title(reinterpret_cast<const char*>(blob.data() + offset), title_len);
        offset += title_len;
        
        // Read username
        if (offset + 2 > blob.size()) break;
        uint16_t user_len;
        std::memcpy(&user_len, blob.data() + offset, 2);
        offset += 2;
        
        if (offset + user_len > blob.size()) break;
        std::string username(reinterpret_cast<const char*>(blob.data() + offset), user_len);
        offset += user_len;
        
        // Read password
        if (offset + 2 > blob.size()) break;
        uint16_t pass_len;
        std::memcpy(&pass_len, blob.data() + offset, 2);
        offset += 2;
        
        if (offset + pass_len > blob.size()) break;
        std::string password(reinterpret_cast<const char*>(blob.data() + offset), pass_len);
        offset += pass_len;
        
        credentials.push_back(Credential(title.c_str(), username.c_str(), password.c_str()));
    }
    
    printf("Loaded %zu credentials from flash\n", credentials.size());
    return true;
}

bool CredentialManager::save_to_flash() {
    std::vector<uint8_t> blob;
    
    // Write count
    uint32_t count = credentials.size();
    blob.insert(blob.end(), 
                reinterpret_cast<uint8_t*>(&count), 
                reinterpret_cast<uint8_t*>(&count) + 4);
    
    // Write each credential
    for (const auto& cred : credentials) {
        // Title
        uint16_t title_len = cred.title.length();
        blob.insert(blob.end(), 
                    reinterpret_cast<uint8_t*>(&title_len), 
                    reinterpret_cast<uint8_t*>(&title_len) + 2);
        blob.insert(blob.end(), cred.title.begin(), cred.title.end());
        
        // Username
        uint16_t user_len = cred.username.length();
        blob.insert(blob.end(), 
                    reinterpret_cast<uint8_t*>(&user_len), 
                    reinterpret_cast<uint8_t*>(&user_len) + 2);
        blob.insert(blob.end(), cred.username.begin(), cred.username.end());
        
        // Password
        uint16_t pass_len = cred.password.length();
        blob.insert(blob.end(), 
                    reinterpret_cast<uint8_t*>(&pass_len), 
                    reinterpret_cast<uint8_t*>(&pass_len) + 2);
        blob.insert(blob.end(), cred.password.begin(), cred.password.end());
    }
    
    printf("Saving %zu credentials to flash (%zu bytes)\n", credentials.size(), blob.size());
    return flash_store::write_blob(blob);
}
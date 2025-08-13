#pragma once
#include <vector>
#include <string>
#include <stdint.h>

/* // A simple credential record
struct Login
{
    std::string username;
    std::string password;
}; */

namespace secure_store
{
    // Load encrypted logins from internal flash. Returns true on success.
    bool load(const std::string &master_password, std::vector<Login> &out_logins);

    // Save encrypted logins to internal flash. Returns true on success.
    bool save(const std::string &master_password, const std::vector<Login> &logins);
}

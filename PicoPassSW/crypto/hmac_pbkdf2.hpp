#pragma once
#include <vector>
#include <string>
#include "../picosha2.hpp"

namespace crypto
{
    std::vector<uint8_t> hmac_sha256(const std::vector<uint8_t> &key, const std::vector<uint8_t> &msg);
    std::vector<uint8_t> pbkdf2_hmac_sha256(const std::string &pass,
                                            const std::vector<uint8_t> &salt,
                                            uint32_t iterations,
                                            size_t dkLen);
}

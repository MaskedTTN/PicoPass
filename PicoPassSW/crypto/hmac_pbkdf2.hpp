#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include "../picosha2.hpp" // picosha2 is already in PicoPassSW/

namespace crypto
{

    // HMAC-SHA256(key, msg) -> 32 bytes
    inline std::vector<uint8_t> hmac_sha256(const std::vector<uint8_t> &key_in,
                                            const std::vector<uint8_t> &msg)
    {
        const size_t blocksize = 64;
        std::vector<uint8_t> key = key_in;

        // If key is longer than blocksize, hash it
        if (key.size() > blocksize)
        {
            std::vector<uint8_t> hashed(32);
            picosha2::hash256(key.begin(), key.end(), hashed.begin(), hashed.end());
            key = hashed;
        }
        // Pad key to blocksize with zeros
        if (key.size() < blocksize)
            key.resize(blocksize, 0x00);

        // ipad/opad
        std::vector<uint8_t> i_key(blocksize), o_key(blocksize);
        for (size_t i = 0; i < blocksize; ++i)
        {
            i_key[i] = key[i] ^ 0x36;
            o_key[i] = key[i] ^ 0x5c;
        }

        // inner = H( (K ^ ipad) || msg )
        std::vector<uint8_t> inner;
        inner.reserve(blocksize + msg.size());
        inner.insert(inner.end(), i_key.begin(), i_key.end());
        inner.insert(inner.end(), msg.begin(), msg.end());
        std::vector<uint8_t> inner_hash(32);
        picosha2::hash256(inner.begin(), inner.end(), inner_hash.begin(), inner_hash.end());

        // outer = H( (K ^ opad) || inner_hash )
        std::vector<uint8_t> outer;
        outer.reserve(blocksize + inner_hash.size());
        outer.insert(outer.end(), o_key.begin(), o_key.end());
        outer.insert(outer.end(), inner_hash.begin(), inner_hash.end());

        std::vector<uint8_t> out(32);
        picosha2::hash256(outer.begin(), outer.end(), out.begin(), out.end());
        return out;
    }

    // PBKDF2-HMAC-SHA256(password, salt, iterations, dkLen) -> dkLen bytes
    inline std::vector<uint8_t> pbkdf2_hmac_sha256(const std::string &password,
                                                   const std::vector<uint8_t> &salt,
                                                   uint32_t iterations,
                                                   size_t dkLen)
    {
        std::vector<uint8_t> pass(password.begin(), password.end());
        std::vector<uint8_t> dk(dkLen);
        const uint32_t hLen = 32;
        const uint32_t blocks = (dkLen + hLen - 1) / hLen;

        size_t offset = 0;
        for (uint32_t i = 1; i <= blocks; ++i)
        {
            // U1 = HMAC(P, S || INT_32_BE(i))
            std::vector<uint8_t> msg(salt);
            msg.push_back((i >> 24) & 0xFF);
            msg.push_back((i >> 16) & 0xFF);
            msg.push_back((i >> 8) & 0xFF);
            msg.push_back(i & 0xFF);

            std::vector<uint8_t> u = hmac_sha256(pass, msg);
            std::vector<uint8_t> t = u;

            for (uint32_t j = 1; j < iterations; ++j)
            {
                u = hmac_sha256(pass, u);
                for (size_t k = 0; k < hLen; ++k)
                    t[k] ^= u[k];
            }

            const size_t take = std::min<size_t>(hLen, dkLen - offset);
            std::copy(t.begin(), t.begin() + take, dk.begin() + offset);
            offset += take;
        }
        return dk;
    }

} // namespace crypto

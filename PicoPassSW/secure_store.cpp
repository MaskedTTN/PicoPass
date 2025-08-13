#include "secure_store.hpp"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <stdint.h>

#include "pico/time.h"            // for to_ms_since_boot (weak entropy fallback)
#include "flash_store.hpp"        // flash read/write backend
#include "crypto/hmac_pbkdf2.hpp" // HMAC + PBKDF2 built on picosha2.hpp

// -----------------------------------------------------------------------------
// File format stored in flash_store blob (all little endian where applicable):
//   magic:    'P','P','S','S'  (4 bytes)
//   version:  0x01              (1 byte)
//   salt:     16 bytes
//   nonce:    16 bytes
//   iter:     uint32_le         (PBKDF2 iterations)
//   payload_len: uint32_le
//   payload:  encrypted serialized logins
//   tag:      HMAC-SHA256 over (magic..payload) with derived key (32 bytes)
// -----------------------------------------------------------------------------

namespace
{

    // Weak random source (replace with a better source if available).
    static void random_bytes(uint8_t *buf, size_t n)
    {
        uint32_t s = to_ms_since_boot(get_absolute_time()); // time-based seed
        for (size_t i = 0; i < n; i++)
        {
            // xorshift32
            s ^= s << 13;
            s ^= s >> 17;
            s ^= s << 5;
            buf[i] = static_cast<uint8_t>(s & 0xFF);
        }
    }

    // Serialize vector<Login> into a compact binary blob.
    static void serialize(const std::vector<Login> &logins, std::vector<uint8_t> &out)
    {
        out.clear();
        auto put32 = [&](uint32_t v)
        {
            out.push_back(static_cast<uint8_t>(v & 0xFF));
            out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
            out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
            out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
        };
        put32(static_cast<uint32_t>(logins.size()));
        for (const auto &L : logins)
        {
            put32(static_cast<uint32_t>(L.username.size()));
            out.insert(out.end(), L.username.begin(), L.username.end());
            put32(static_cast<uint32_t>(L.password.size()));
            out.insert(out.end(), L.password.begin(), L.password.end());
        }
    }

    // Deserialize vector<Login> from binary blob.
    static bool deserialize(const std::vector<uint8_t> &in, std::vector<Login> &out)
    {
        out.clear();
        size_t i = 0;
        auto get32 = [&](uint32_t &v) -> bool
        {
            if (i + 4 > in.size())
                return false;
            v = static_cast<uint32_t>(in[i]) |
                (static_cast<uint32_t>(in[i + 1]) << 8) |
                (static_cast<uint32_t>(in[i + 2]) << 16) |
                (static_cast<uint32_t>(in[i + 3]) << 24);
            i += 4;
            return true;
        };

        uint32_t count = 0;
        if (!get32(count))
            return false;

        for (uint32_t k = 0; k < count; k++)
        {
            uint32_t usz = 0, psz = 0;
            if (!get32(usz) || i + usz > in.size())
                return false;
            std::string u(reinterpret_cast<const char *>(&in[i]), usz);
            i += usz;

            if (!get32(psz) || i + psz > in.size())
                return false;
            std::string p(reinterpret_cast<const char *>(&in[i]), psz);
            i += psz;

            out.push_back({u, p});
        }
        return true;
    }

    // Build a keystream using HMAC-SHA256(key, nonce || counter) blocks
    static void hmac_keystream(const std::vector<uint8_t> &key,
                               const std::vector<uint8_t> &nonce,
                               uint8_t *out, size_t n)
    {
        uint32_t counter = 0;
        size_t produced = 0;
        while (produced < n)
        {
            std::vector<uint8_t> msg(nonce);
            msg.push_back(static_cast<uint8_t>((counter >> 24) & 0xFF));
            msg.push_back(static_cast<uint8_t>((counter >> 16) & 0xFF));
            msg.push_back(static_cast<uint8_t>((counter >> 8) & 0xFF));
            msg.push_back(static_cast<uint8_t>(counter & 0xFF));

            auto block = crypto::hmac_sha256(key, msg); // 32 bytes
            const size_t take = std::min(block.size(), n - produced);
            std::memcpy(out + produced, block.data(), take);
            produced += take;
            counter++;
        }
    }

    static void xor_bytes(uint8_t *data, const uint8_t *ks, size_t n)
    {
        for (size_t i = 0; i < n; i++)
            data[i] ^= ks[i];
    }

} // namespace

// PUBLIC API
namespace secure_store
{

    bool save(const std::string &master_password, const std::vector<Login> &logins)
    {
        // Derive key
        std::vector<uint8_t> salt(16), nonce(16);
        random_bytes(salt.data(), salt.size());
        random_bytes(nonce.data(), nonce.size());
        const uint32_t iterations = 100000;

        const auto key = crypto::pbkdf2_hmac_sha256(master_password, salt, iterations, 32);

        // Serialize and encrypt
        std::vector<uint8_t> payload;
        serialize(logins, payload);

        std::vector<uint8_t> enc(payload);      // copy
        std::vector<uint8_t> ks(enc.size(), 0); // keystream
        if (!enc.empty())
        {
            hmac_keystream(key, nonce, ks.data(), ks.size());
            xor_bytes(enc.data(), ks.data(), enc.size());
        }

        // Build header + ciphertext
        std::vector<uint8_t> file;
        file.insert(file.end(), {'P', 'P', 'S', 'S'});       // magic
        file.push_back(0x01);                                // version
        file.insert(file.end(), salt.begin(), salt.end());   // 16
        file.insert(file.end(), nonce.begin(), nonce.end()); // 16

        // iterations (u32 LE)
        file.push_back(static_cast<uint8_t>(iterations & 0xFF));
        file.push_back(static_cast<uint8_t>((iterations >> 8) & 0xFF));
        file.push_back(static_cast<uint8_t>((iterations >> 16) & 0xFF));
        file.push_back(static_cast<uint8_t>((iterations >> 24) & 0xFF));

        // payload length (u32 LE)
        const uint32_t plen = static_cast<uint32_t>(enc.size());
        file.push_back(static_cast<uint8_t>(plen & 0xFF));
        file.push_back(static_cast<uint8_t>((plen >> 8) & 0xFF));
        file.push_back(static_cast<uint8_t>((plen >> 16) & 0xFF));
        file.push_back(static_cast<uint8_t>((plen >> 24) & 0xFF));

        // ciphertext
        file.insert(file.end(), enc.begin(), enc.end());

        // Tag = HMAC(key, header+ciphertext)
        const auto tag = crypto::hmac_sha256(key, file);
        file.insert(file.end(), tag.begin(), tag.end()); // 32

        // Write to flash
        return flash_store::write_blob(file);
    }

    bool load(const std::string &master_password, std::vector<Login> &out_logins)
    {
        std::vector<uint8_t> buf;
        if (!flash_store::read_blob(buf))
            return false;

        size_t i = 0;
        auto need = [&](size_t k) -> bool
        { return (i + k) <= buf.size(); };

        // magic
        if (!need(4))
            return false;
        if (buf[i] != 'P' || buf[i + 1] != 'P' || buf[i + 2] != 'S' || buf[i + 3] != 'S')
            return false;
        i += 4;

        // version
        if (!need(1))
            return false;
        const uint8_t version = buf[i++];
        if (version != 0x01)
            return false;

        // salt, nonce
        if (!need(16))
            return false;
        std::vector<uint8_t> salt(buf.begin() + i, buf.begin() + i + 16);
        i += 16;

        if (!need(16))
            return false;
        std::vector<uint8_t> nonce(buf.begin() + i, buf.begin() + i + 16);
        i += 16;

        // iterations
        if (!need(4))
            return false;
        const uint32_t iterations =
            static_cast<uint32_t>(buf[i]) |
            (static_cast<uint32_t>(buf[i + 1]) << 8) |
            (static_cast<uint32_t>(buf[i + 2]) << 16) |
            (static_cast<uint32_t>(buf[i + 3]) << 24);
        i += 4;

        // payload length
        if (!need(4))
            return false;
        const uint32_t plen =
            static_cast<uint32_t>(buf[i]) |
            (static_cast<uint32_t>(buf[i + 1]) << 8) |
            (static_cast<uint32_t>(buf[i + 2]) << 16) |
            (static_cast<uint32_t>(buf[i + 3]) << 24);
        i += 4;

        // ciphertext + tag
        if (!need(plen + 32))
            return false;
        std::vector<uint8_t> enc(buf.begin() + i, buf.begin() + i + plen);
        i += plen;
        std::vector<uint8_t> tag(buf.begin() + i, buf.begin() + i + 32);
        i += 32;

        // derive key
        const auto key = crypto::pbkdf2_hmac_sha256(master_password, salt, iterations, 32);

        // verify tag
        std::vector<uint8_t> header(buf.begin(), buf.begin() + (buf.size() - 32));
        const auto tag2 = crypto::hmac_sha256(key, header);
        if (tag != tag2)
            return false;

        // decrypt
        std::vector<uint8_t> payload(enc);
        if (!payload.empty())
        {
            std::vector<uint8_t> ks(payload.size(), 0);
            hmac_keystream(key, nonce, ks.data(), ks.size());
            xor_bytes(payload.data(), ks.data(), payload.size());
        }

        return deserialize(payload, out_logins);
    }

} // namespace secure_store

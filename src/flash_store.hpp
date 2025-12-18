#pragma once
#include <vector>
#include <cstdint>

// A simple flash-backed key/value “blob” store for one encrypted payload.
namespace flash_store
{
    // Returns true if something valid is stored.
    bool read_blob(std::vector<uint8_t> &out);

    // Overwrites flash with the new blob.
    bool write_blob(const std::vector<uint8_t> &data);
}
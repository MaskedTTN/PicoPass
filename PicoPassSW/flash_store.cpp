#include "flash_store.hpp"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include <cstring>
#include <algorithm>

// Reserve 64KB at the end of flash for data.
// Make sure your app does not exceed (flash_size - FLASH_STORE_SIZE).
#ifndef FLASH_STORE_SIZE
#define FLASH_STORE_SIZE (64 * 1024)
#endif

// On most Pico boards the XIP flash is mapped at 0x10000000.
#ifndef XIP_BASE
#define XIP_BASE 0x10000000u
#endif

// Total flash size in bytes; set via compile definitions or CMake cache.
// If not set, assume 2MB (RP2040 default boards).
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
#endif

// We store our blob in the last region of flash.
static constexpr uint32_t FLASH_STORE_OFFSET = PICO_FLASH_SIZE_BYTES - FLASH_STORE_SIZE;
static constexpr uint32_t ERASE_SECTOR = 4096;
static constexpr uint32_t PROG_PAGE = 256;

static const uint8_t *flash_ptr()
{
    return reinterpret_cast<const uint8_t *>(XIP_BASE + FLASH_STORE_OFFSET);
}

// Very small header: magic + u32 length.
struct Header
{
    uint8_t magic[4]; // "PPSS"
    uint32_t length;  // bytes of payload following header
};

bool flash_store::read_blob(std::vector<uint8_t> &out)
{
    const auto *p = flash_ptr();
    Header h{};
    std::memcpy(&h, p, sizeof(h));
    if (std::memcmp(h.magic, "PPSS", 4) != 0)
        return false;
    if (h.length == 0 || h.length > FLASH_STORE_SIZE - sizeof(Header))
        return false;
    out.resize(h.length);
    std::memcpy(out.data(), p + sizeof(Header), h.length);
    return true;
}

bool flash_store::write_blob(const std::vector<uint8_t> &data)
{
    if (data.size() > FLASH_STORE_SIZE - sizeof(Header))
        return false;

    // Build staging buffer aligned to pages
    std::vector<uint8_t> img(sizeof(Header) + data.size(), 0xFF);
    Header h{{'P', 'P', 'S', 'S'}, static_cast<uint32_t>(data.size())};
    std::memcpy(img.data(), &h, sizeof(h));
    std::memcpy(img.data() + sizeof(Header), data.data(), data.size());

    // Erase + program (interrupts must be disabled during flash ops)
    uint32_t ints = save_and_disable_interrupts();

    // Erase the whole reserved region
    for (uint32_t off = 0; off < FLASH_STORE_SIZE; off += ERASE_SECTOR)
    {
        flash_range_erase(FLASH_STORE_OFFSET + off, ERASE_SECTOR);
    }
    // Program in PROG_PAGE chunks
    for (uint32_t off = 0; off < img.size(); off += PROG_PAGE)
    {
        uint32_t chunk = std::min<uint32_t>(PROG_PAGE, img.size() - off);
        uint8_t page[PROG_PAGE];
        std::memset(page, 0xFF, PROG_PAGE);
        std::memcpy(page, img.data() + off, chunk);
        flash_range_program(FLASH_STORE_OFFSET + off, page, PROG_PAGE);
    }

    restore_interrupts(ints);
    return true;
}

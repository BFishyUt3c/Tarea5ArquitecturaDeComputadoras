#pragma once

#include <cstdint>
#include <vector>

class Memory {
private:
    static constexpr uint32_t MEMORY_SIZE = 0x10000000;  // 256MB address space
    static constexpr uint32_t MEMORY_MASK = MEMORY_SIZE - 1;
    std::vector<uint8_t> data;

public:
    Memory();
    uint8_t read_byte(uint32_t addr) const;
    uint16_t read_half(uint32_t addr) const;
    uint32_t read_word(uint32_t addr) const;
    void write_byte(uint32_t addr, uint8_t value);
    void write_half(uint32_t addr, uint16_t value);
    void write_word(uint32_t addr, uint32_t value);
    const uint8_t* raw_data() const { return data.data(); }
    uint8_t* raw_data() { return data.data(); }
};

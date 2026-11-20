#include "mem.hpp"
#include <stdexcept>

Memory::Memory() : data(std::vector<uint8_t>(MEMORY_SIZE, 0)) {}

uint8_t Memory::read_byte(uint32_t addr) const {
    if (addr > MEMORY_MASK) {
        throw std::runtime_error("Dirección de memoria fuera de rango");
    }
    return data[addr];
}

uint16_t Memory::read_half(uint32_t addr) const {
    if (addr > MEMORY_MASK - 1) {
        throw std::runtime_error("Dirección de memoria fuera de rango para lectura halfword");
    }
    return (static_cast<uint16_t>(read_byte(addr)) |
            static_cast<uint16_t>(read_byte(addr + 1)) << 8);
}

uint32_t Memory::read_word(uint32_t addr) const {
    if (addr > MEMORY_MASK - 3) {
        throw std::runtime_error("Dirección de memoria fuera de rango para lectura word");
    }
    return (static_cast<uint32_t>(read_byte(addr)) |
            static_cast<uint32_t>(read_byte(addr + 1)) << 8 |
            static_cast<uint32_t>(read_byte(addr + 2)) << 16 |
            static_cast<uint32_t>(read_byte(addr + 3)) << 24);
}

void Memory::write_byte(uint32_t addr, uint8_t value) {
    if (addr > MEMORY_MASK) {
        throw std::runtime_error("Dirección de memoria fuera de rango");
    }
    data[addr] = value;
}

void Memory::write_half(uint32_t addr, uint16_t value) {
    if (addr > MEMORY_MASK - 1) {
        throw std::runtime_error("Dirección de memoria fuera de rango para escritura halfword");
    }
    write_byte(addr, static_cast<uint8_t>(value & 0xFF));
    write_byte(addr + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
}

void Memory::write_word(uint32_t addr, uint32_t value) {
    if (addr > MEMORY_MASK - 3) {
        throw std::runtime_error("Dirección de memoria fuera de rango para escritura word");
    }
    write_byte(addr, static_cast<uint8_t>(value & 0xFF));
    write_byte(addr + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
    write_byte(addr + 2, static_cast<uint8_t>((value >> 16) & 0xFF));
    write_byte(addr + 3, static_cast<uint8_t>((value >> 24) & 0xFF));
}
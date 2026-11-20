#pragma once

#include <cstdint>
#include <string>
#include "../mem/mem.hpp"
#include "../loader/loader.hpp"

class Cpu {
private:
    bool halted = false;
    uint32_t pc = 0;
    uint32_t regs[32] = {};
    Memory mem;

public:
    Cpu();
    bool load_from_file(const std::string& filename);
    bool is_halted() const;
    uint32_t get_pc() const;
    uint32_t get_reg(uint32_t idx) const;
    void set_reg(uint32_t idx, uint32_t value);
    uint32_t get_instruction() const;
    Memory& get_memory();
    void step();
};
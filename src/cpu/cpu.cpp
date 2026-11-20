#include "cpu.hpp"
#include <iostream>
#include <stdexcept>

namespace {

static inline int32_t sext_12(uint32_t x) { return ((int32_t)(x << 20)) >> 20; }
static inline int32_t sext_13(uint32_t x) { return ((int32_t)(x << 19)) >> 19; }
static inline int32_t sext_21(uint32_t x) { return ((int32_t)(x << 11)) >> 11; }

static inline uint32_t imm_i(uint32_t instr) { return (instr >> 20) & 0xFFF; }
static inline uint32_t imm_s(uint32_t instr) {
    return (((instr >> 25) & 0x3F) << 5) | ((instr >> 7) & 0x1F);
}
static inline uint32_t imm_b(uint32_t instr) {
    return (((instr >> 31) & 1) << 12) | (((instr >> 7) & 1) << 11) |
           (((instr >> 25) & 0x3F) << 5) | (((instr >> 8) & 0xF) << 1);
}
static inline uint32_t imm_u(uint32_t instr) { return instr & 0xFFFFF000; }
static inline uint32_t imm_j(uint32_t instr) {
    return ((instr & 0x80000000) >> 11) |   // imm[20]
           (instr & 0x000FF000) |            // imm[19:12]
           ((instr & 0x00100000) >> 9) |     // imm[11]
           ((instr & 0x7FE00000) >> 20);     // imm[10:1]
}

}  // namespace

Cpu::Cpu() : halted(false), pc(0), regs{}, mem() {}

bool Cpu::load_from_file(const std::string& filename) {
    return Loader::load_from_file(mem, filename);
}

bool Cpu::is_halted() const { return halted; }

uint32_t Cpu::get_pc() const { return pc; }

uint32_t Cpu::get_reg(uint32_t idx) const {
    if (idx >= 32) throw std::runtime_error("Registro fuera de rango");
    return (idx == 0) ? 0 : regs[idx];
}

void Cpu::set_reg(uint32_t idx, uint32_t value) {
    if (idx >= 32) throw std::runtime_error("Registro fuera de rango");
    regs[idx] = (idx == 0) ? 0 : value;
}

uint32_t Cpu::get_instruction() const { return mem.read_word(pc); }

Memory& Cpu::get_memory() { return mem; }

void Cpu::step() {
    uint32_t instr = mem.read_word(pc);
    uint32_t next_pc = pc + 4;
    uint32_t new_pc = next_pc;  // por defecto: avanzar

    uint8_t opcode = instr & 0x7F;
    uint8_t rd   = (instr >> 7)  & 0x1F;
    uint8_t f3   = (instr >> 12) & 0x7;
    uint8_t rs1  = (instr >> 15) & 0x1F;
    uint8_t rs2  = (instr >> 20) & 0x1F;
    uint8_t f7   = (instr >> 25) & 0x7F;

    switch (opcode) {
        // ===== R-type =====
        case 0x33: {
            uint32_t s = regs[rs2] & 0x1F;
            switch (f3) {
                case 0x0: regs[rd] = (f7 == 0x00) ? regs[rs1] + regs[rs2]   // ADD
                                                   : regs[rs1] - regs[rs2]; // SUB
                            break;
                case 0x1: regs[rd] = regs[rs1] << s; break;                 // SLL
                case 0x2: regs[rd] = (int32_t)regs[rs1] < (int32_t)regs[rs2]; break; // SLT
                case 0x3: regs[rd] = regs[rs1] < regs[rs2]; break;          // SLTU
                case 0x4: regs[rd] = regs[rs1] ^ regs[rs2]; break;          // XOR
                case 0x5: regs[rd] = (f7 == 0x00) ? regs[rs1] >> s          // SRL
                                                   : (int32_t)regs[rs1] >> s; // SRA
                            break;
                case 0x6: regs[rd] = regs[rs1] | regs[rs2]; break;          // OR
                case 0x7: regs[rd] = regs[rs1] & regs[rs2]; break;          // AND
            }
            break;
        }

        // ===== I-type ALU =====
        case 0x13: {
            uint32_t i = imm_i(instr);
            int32_t simm = sext_12(i);
            uint32_t shamt = i & 0x1F;
            switch (f3) {
                case 0x0: regs[rd] = regs[rs1] + simm; break;               // ADDI
                case 0x1: regs[rd] = regs[rs1] << shamt; break;             // SLLI
                case 0x2: regs[rd] = (int32_t)regs[rs1] < simm; break;      // SLTI
                case 0x3: regs[rd] = regs[rs1] < (uint32_t)simm; break;     // SLTIU
                case 0x4: regs[rd] = regs[rs1] ^ i; break;                  // XORI
                case 0x5: regs[rd] = (f7 == 0x00) ? regs[rs1] >> shamt      // SRLI
                                                   : (int32_t)regs[rs1] >> shamt; // SRAI
                            break;
                case 0x6: regs[rd] = regs[rs1] | i; break;                  // ORI
                case 0x7: regs[rd] = regs[rs1] & i; break;                  // ANDI
            }
            break;
        }

        // ===== Loads =====
        case 0x03: {
            uint32_t addr = regs[rs1] + sext_12(imm_i(instr));
            uint8_t* m = mem.raw_data();
            switch (f3) {
                case 0x0: regs[rd] = (int8_t)m[addr]; break;                // LB
                case 0x1: regs[rd] = (int16_t)mem.read_half(addr); break;   // LH
                case 0x2: regs[rd] = mem.read_word(addr); break;            // LW
                case 0x4: regs[rd] = m[addr]; break;                        // LBU
                case 0x5: regs[rd] = mem.read_half(addr); break;            // LHU
            }
            break;
        }

        // ===== Stores =====
        case 0x23: {
            uint32_t addr = regs[rs1] + sext_12(imm_s(instr));
            uint8_t* m = mem.raw_data();
            switch (f3) {
                case 0x0: m[addr] = regs[rs2] & 0xFF; break;                // SB
                case 0x1: mem.write_half(addr, regs[rs2]); break;           // SH
                case 0x2: mem.write_word(addr, regs[rs2]); break;           // SW
            }
            break;
        }

        // ===== Branches =====
        case 0x63: {
            int32_t off = sext_13(imm_b(instr));
            bool t = false;
            switch (f3) {
                case 0x0: t = (regs[rs1] == regs[rs2]); break;   // BEQ
                case 0x1: t = (regs[rs1] != regs[rs2]); break;   // BNE
                case 0x4: t = (int32_t)regs[rs1] < (int32_t)regs[rs2]; break;  // BLT
                case 0x5: t = (int32_t)regs[rs1] >= (int32_t)regs[rs2]; break; // BGE
                case 0x6: t = regs[rs1] < regs[rs2]; break;      // BLTU
                case 0x7: t = regs[rs1] >= regs[rs2]; break;     // BGEU
            }
            if (t) new_pc = next_pc + off;
            break;
        }

        // ===== JALR =====
        case 0x67: {
            int32_t off = sext_12(imm_i(instr));
            uint32_t t = (regs[rs1] + off) & ~1u;
            regs[rd] = next_pc;
            new_pc = t;
            break;
        }

        // ===== JAL =====
        case 0x6F: {
            regs[rd] = next_pc;
            new_pc = next_pc + sext_21(imm_j(instr));
            break;
        }

        // ===== AUIPC =====
        case 0x17: {
            regs[rd] = pc + imm_u(instr);
            break;
        }

        // ===== LUI =====
        case 0x37: {
            regs[rd] = imm_u(instr);
            break;
        }

        default:
            std::cerr << "Opcode desconocido: 0x" << std::hex << (int)opcode << "\n";
            halted = true;
            break;
    }

    pc = new_pc;
    regs[0] = 0;  // x0 hardwired
}
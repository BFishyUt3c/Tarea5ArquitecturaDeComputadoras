#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdint>
#include "cpu/cpu.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Debe especificar un archivo de entrada.\n";
        return 1;
    }

    Cpu cpu;

    if (!cpu.load_from_file(argv[1])) {
        std::cerr << "No se pudo cargar el archivo.\n";
        return 1;
    }
    std::cout << "\"" << argv[1] << "\" cargado a memoria.\n";

    while (true) {
        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit" || cmd == "quit") {
            std::cout << "See you next time...\n";
            return 0;
        }

        if (cmd == "pc") {
            std::cout << "pc=0x" << std::hex << cpu.get_pc() << std::dec << "\n";
        } else if (cmd == "step") {
            if (cpu.is_halted()) {
                std::cout << "Simulador detenido.\n";
            } else {
                cpu.step();
                std::cout << "pc=0x" << std::hex << cpu.get_pc() << std::dec << "\n";
            }
        } else if (cmd == "run") {
            uint32_t steps = 0;
            const uint32_t MAX_STEPS = 100000;
            while (!cpu.is_halted() && steps < MAX_STEPS) { cpu.step(); ++steps; }
            std::cout << "Ejecutadas " << steps << " instrucciones. pc=0x" << std::hex << cpu.get_pc() << std::dec << "\n";
            if (!cpu.is_halted()) std::cout << "Limite de pasos alcanzado.\n";
        } else if (cmd == "regs") {
            std::string a;
            bool any = false;
            while (iss >> a) {
                if (!a.empty() && (a[0] == 'x' || a[0] == 'X')) a = a.substr(1);
                uint32_t idx = std::stoul(a, nullptr, 0);
                std::cout << "x" << idx << "=0x" << std::hex << cpu.get_reg(idx) << std::dec << "\n";
                any = true;
            }
            if (!any) {
                for (int i = 0; i < 32; ++i) {
                    std::cout << "x" << i << "=0x" << std::hex << cpu.get_reg(i) << std::dec << "\n";
                }
            }
        } else if (cmd == "mem") {
            std::string arg1, arg2;
            iss >> arg1 >> arg2;
            if (arg1.empty() || arg2.empty()) {
                std::cout << "Uso: mem <addr_ini> <addr_fin>\n";
            } else {
                uint32_t start = std::stoul(arg1, nullptr, 16);
                uint32_t end = std::stoul(arg2, nullptr, 16);
                uint8_t* data = cpu.get_memory().raw_data();
                for (uint32_t addr = start; addr <= end; ++addr) {
                    if (addr % 4 == 0)
                        std::cout << "0x" << std::hex << addr << ": ";
                    std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)data[addr] << " ";
                    if (addr % 4 == 3)
                        std::cout << "\n";
                }
                if (end % 4 != 3)
                    std::cout << "\n";
                std::cout << std::dec;
            }
        } else if (cmd == "help") {
            std::cout << "Comandos:\n";
            std::cout << "  pc          - Mostrar PC\n";
            std::cout << "  step        - Ejecutar instrucción\n";
            std::cout << "  regs [x..]  - Mostrar registros (todos o especificos)\n";
            std::cout << "  mem <ini> <fin> - Mostrar memoria (direcciones en hex)\n";
            std::cout << "  run         - Ejecutar hasta detenerse\n";
            std::cout << "  exit/quit   - Salir\n";
        } else {
            std::cout << "Comando no reconocido. Escriba 'help'.\n";
        }
    }

    return 0;
}
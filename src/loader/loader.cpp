#include "loader.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>

bool Loader::load_from_file(Memory& mem, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    // Read whole file into a buffer (raw binary)
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
    // Load starting at address 0x00000000
    for (size_t i = 0; i < buffer.size(); ++i) {
        mem.write_byte(static_cast<uint32_t>(i), buffer[i]);
    }
    return true;
}
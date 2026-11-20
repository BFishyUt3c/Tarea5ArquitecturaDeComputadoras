#pragma once

#include <string>
#include "../mem/mem.hpp"

class Loader {
public:
    static bool load_from_file(Memory& mem, const std::string& filename);
};
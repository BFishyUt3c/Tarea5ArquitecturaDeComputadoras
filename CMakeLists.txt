cmake_minimum_required(VERSION 3.10)
project(riscv-sim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_executable(riscv-sim
    src/main.cpp
    src/cpu/cpu.cpp src/cpu/cpu.hpp
    src/mem/mem.cpp src/mem/mem.hpp
    src/loader/loader.cpp src/loader/loader.hpp
)

install(TARGETS riscv-sim DESTINATION bin)

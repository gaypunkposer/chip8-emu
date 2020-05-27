cmake_minimum_required(VERSION 3.10)
project(chip8)

set(SDL2_INCLUDE_DIRS "C:/SDL2/include")
set(SDL2_LIBRARIES "C:/SDL2/lib/x64/SDL2.lib;C:/SDL2/lib/x64/SDL2main.lib")
set(CMAKE_CXX_STANDARD 14)

include_directories(${SDL2_INCLUDE_DIRS})
set(chip8_ARCHITECTURE "x64")
set(CMAKE_SYSTEM_PROCESSOR x64)
set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(chip8 main.cpp chip8.cpp chip8.h)
target_link_libraries(chip8 ${SDL2_LIBRARIES})
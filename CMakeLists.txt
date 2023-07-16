cmake_minimum_required(VERSION 3.13)


include(pico-sdk/pico_sdk_init.cmake)
project(homeds_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


pico_sdk_init()

add_subdirectory(src)


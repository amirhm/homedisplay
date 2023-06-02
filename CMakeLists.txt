cmake_minimum_required(VERSION 3.13)


include(pico-sdk/pico_sdk_init.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(WIFI_SSID $ENV{WIFI_SSID})
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

pico_sdk_init()

add_subdirectory(src)


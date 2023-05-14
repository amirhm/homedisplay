cmake_minimum_required(VERSION 3.13)

#include(../pico-sdk/pico_sdk_init.cmake)
include(../pico-sdk/external/pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(test
  test.c
)
target_link_libraries(test pico_stdlib hardware_adc hardware_gpio hardware_pwm hardware_spi)
pico_enable_stdio_usb(test 0)
pico_enable_stdio_uart(test 1)


pico_add_extra_outputs(test)


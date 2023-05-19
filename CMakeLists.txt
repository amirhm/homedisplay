cmake_minimum_required(VERSION 3.13)

#include(../pico-sdk/pico_sdk_init.cmake)
include(../pico-sdk/external/pico_sdk_import.cmake)

project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(test
  main.c
  scd4x.c
  st7789.c
  ntp_client.c
)

target_include_directories(test PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
)
target_link_libraries(test pico_stdlib 
        hardware_adc
        hardware_gpio
        hardware_pwm
        hardware_spi
        hardware_i2c 
        pico_cyw43_arch_lwip_threadsafe_background)
pico_enable_stdio_usb(test 0)
pico_enable_stdio_uart(test 1)


pico_add_extra_outputs(test)


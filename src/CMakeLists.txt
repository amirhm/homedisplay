add_executable(test
  main.c
  scd4x.c
  st7789.c
  ntp_client.c
)

set(WIFI_SSID $ENV{WIFI_SSID})
set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})

target_compile_definitions(test PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
message("WIFI SSID is: ${WIFI_SSID}")

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
        hardware_rtc
        pico_cyw43_arch_lwip_threadsafe_background)

pico_enable_stdio_usb(test 0)
pico_enable_stdio_uart(test 1)


pico_add_extra_outputs(test)


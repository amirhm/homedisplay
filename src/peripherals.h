#ifndef _I2C_H
#define _I2C_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/rtc.h"

typedef struct I2C_MODULE{
    i2c_inst_t* inst;
    int sda_pin;
    int scl_pin;
    int clk_freq_KHz;
} I2C_MODULE;


typedef struct SPI_MODULE{
    spi_inst_t* inst;
    int tx_pin;
    int rx_pin;
    int sck_pin;
    int csn_pin;
    int clk_freq_MHz;
} SPI_MODULE;


typedef struct ST7789{
    SPI_MODULE* spi_module;
    int reset;
    int dc;
    int blk;
} ST7789;

extern ST7789 st7789;
extern I2C_MODULE i2c_moudle;
extern SPI_MODULE spi_module;

int init_i2c();
int init_st7789();
int init_peripherals();
#endif
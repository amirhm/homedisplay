#ifndef _ST7789_H
#define _ST7789_H
#include <stdio.h>
#ifndef TEST
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "peripherals.h"
#endif
typedef enum CMD{
	SWRESET     = 0x01,
	INVON = 0x21,
	INVOFF = 0x20,
	SLPOUT = 0x11,
	DISPOFF = 0x28,
	DISPON = 0x29,
	CASET = 0x2a,
	RASET = 0x2b,
	TEON = 0x35,
	MADCTL = 0x36,
	COLMOD = 0x3A,
	RAMWR = 0x2c
} CMD;


int lcd_set_raset(uint16_t raset_ys, uint16_t raset_ye);
int lcd_set_caset(uint16_t caset_xs, uint16_t caset_xe);
int display_brightness(int value);
int write_buffer(uint8_t reg, uint8_t* buffer, int len);
#endif
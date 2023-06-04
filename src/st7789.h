
#ifndef _ST7789_H
#define _ST7789_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"


#define FRMWIDTH 240
#define FRMHEIGHT 300

typedef enum CMD{
	SWRESET     = 0x01,
	INVON = 0x21,
	INVOFF = 0x20,
	SLPOUT = 0x11,
	DISPON = 0x29,
	CASET = 0x2a,
	RASET = 0x2b,
	TEON = 0x35,
	MADCTL = 0x36,
	COLMOD = 0x3A,
	RAMWR = 0x2c
} CMD;

typedef struct DisplayInfo{
	bool update_weather;
	bool update_time;
	bool update_cnt;
	bool update;
	int LineHeight;
	int ML;
} DisplayInfo;

DisplayInfo display;

int lcd_set_raset(uint16_t raset_ys, uint16_t raset_ye);
int lcd_set_caset(uint16_t caset_xs, uint16_t caset_xe);
int init_display();
int fill_display(uint16_t color);
int fill_display_gradient();
int write_character(uint16_t x, uint16_t y, uint16_t color);
int update_display();
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
int write_string(uint16_t x, uint16_t y, char* string, uint16_t fcolor, int ds);
int display_brightness(int value);
#endif
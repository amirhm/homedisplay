
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

#define FRMWIDTH 240
#define FRMHEIGHT 300


void init_gpio();
int lcd_set_raset(uint16_t raset_ys, uint16_t raset_ye);
int lcd_set_caset(uint16_t caset_xs, uint16_t caset_xe);
int init_display();
int fill_display(uint16_t color);
int fill_display_gradient();
int write_character(uint16_t x, uint16_t y, uint16_t color);
int update_display();
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
int write_number(uint16_t x, uint16_t y, uint16_t number);
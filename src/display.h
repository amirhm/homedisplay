#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <time.h>

extern const unsigned char Ubuntu[9124];
#ifdef TEST
#include "stdbool.h" 
#include "stdint.h" 



typedef struct{
	int16_t year;
	int8_t month;
	int8_t day;
	int8_t dotw;
	int8_t hour;
	int8_t min;
	int8_t sec;
}datetime_t;
#else
#include "pico/util/datetime.h"
#endif

#define LARGDISPLAY

#ifdef LARGDISPLAY
#define FRMWIDTH 480
#define FRMHEIGHT 320
#else
#define FRMWIDTH 240
#define FRMHEIGHT 300
#endif
#define FRMHEIGHT_S  40
typedef struct DisplayConfig{
	uint16_t day_background;
	uint16_t day_fontcolor;

	uint16_t day_brightness;
	uint16_t night_brightness;
	uint16_t night_background;
	uint16_t night_fontcolor;
} DisplayConfig;

typedef struct DisplayInfo{
	bool update_weather;
	bool update_time;
	bool update_cnt;
	bool update;
	int LineHeight;
	int ML;
	int CO2;
	int TEMP;
	int RH;
	int TIME;
	int DATE;
	int CNT;
} DisplayInfo;

typedef struct DayCnt{
	int week;
	int day;
	time_t start;
}DayCnt;

typedef struct SensorData{
	uint16_t co2_raw;
	uint16_t temperature_raw;
	uint16_t humidity_raw;
	float temperature;
	float humidity;
} SensorData;
int display_time_update(time_t utc_time, datetime_t t, DayCnt day_cnt);
int display_info_update(SensorData weather);
int display_task(SensorData weather, DayCnt day_cnt, time_t utc, datetime_t dt);
int init_disp();
extern DisplayInfo display;
int init_display();
int fill_display(uint16_t color);
int fill_display_gradient();
int write_character(uint16_t x, uint16_t y, uint16_t color);
int update_display();
int write_string(uint16_t x, uint16_t y, char* string, uint16_t fcolor, int ds);
int write_line(uint16_t line_num, char* string, uint16_t bgcolor, uint16_t fcolor, int ds);
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
#ifdef TEST

int display_brightness(int value);
static int write_buffer(uint8_t reg, uint8_t* buffer, int len);
int lcd_set_caset(uint16_t caset_xs, uint16_t caset_xe);
int lcd_set_raset(uint16_t raset_ys, uint16_t raset_ye);

#endif


#endif
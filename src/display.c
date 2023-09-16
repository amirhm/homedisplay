#include "display.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "st7789.h"
uint16_t rcolor;
uint16_t gcolor;
bool day;

const char* month_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//const char* day_str[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

DisplayConfig dcfg;
DisplayInfo display;

static uint16_t color565(uint8_t r, uint8_t g, uint8_t b){
	return (((g & 0x1c) << 3) + (b & 0xf8 ) << 5) + (((r & 0xf8)) + ((g & 0xe0) >> 5));
}

int init_disp(){
	display.CO2 = 4;
	display.TEMP = 5;
	display.RH = 6;
	display.TIME = 0;
	display.DATE = 2;
	display.CNT = 8;
	display.LineHeight = 32;
	rcolor = color565(0xe6,0x39, 0x46);
	gcolor = color565(34, 179, 34);
	dcfg.day_background=color565(220, 220, 220);
	dcfg.day_fontcolor=color565(127, 127, 127);
	dcfg.night_background=color565(0,0,0);
	dcfg.night_fontcolor=color565(127, 127, 127);
	dcfg.day_brightness=0xf000;
	dcfg.night_brightness=0xffff;
	return 0;
}

int display_time_update(time_t utc_time, datetime_t t, DayCnt day_cnt){
	char string[20];
	uint16_t fcolor = (day) ? dcfg.day_fontcolor: dcfg.night_fontcolor;
	uint16_t bgcolor = (day) ? dcfg.day_background : dcfg.night_background;

	struct tm *utc = gmtime(&utc_time);
	sprintf(string, "%s %02d %s", day_str[t.dotw], t.day , month_str[t.month - 1]);
	write_line(display.TIME , string, bgcolor, fcolor, 1);

	sprintf(string, "  %02d:%02d", t.hour , t.min);
	write_line(display.DATE , string, bgcolor, fcolor, 1);

	sprintf(string, " %02dW %dD ~", day_cnt.week, day_cnt.day);
	write_line(display.CNT , string, bgcolor, fcolor, 1);
	display.update_time = false;
}

int display_info_update(SensorData weather){
	char string[20];
	uint16_t fcolor = (day) ? dcfg.day_fontcolor: dcfg.night_fontcolor;
	uint16_t bgcolor = (day) ? dcfg.day_background : dcfg.night_background;
	printf("CO2: %d\n", weather.co2_raw);
	sprintf(string, "CO2: %d", weather.co2_raw);
	write_line(display.CO2, string, bgcolor, (weather.co2_raw < 1200)? gcolor: rcolor, 1);
	sprintf(string, "TMP: %2.1f", weather.temperature);
	write_line(display.TEMP, string, bgcolor, fcolor, 1);
	sprintf(string, "RH: %2.1f%%", weather.humidity);
	write_line(display.RH, string, bgcolor, fcolor, 1);
	display.update_weather = false;
}

int display_fill_empty_lines(){

	uint16_t fcolor = (day) ? dcfg.day_fontcolor: dcfg.night_fontcolor;
	uint16_t bgcolor = (day) ? dcfg.day_background : dcfg.night_background;
	char string[20]="\0";
	write_line(1, string, bgcolor, fcolor, 1);
	write_line(3, string, bgcolor, fcolor, 1);
	write_line(7, string, bgcolor, fcolor, 1);
	write_line(9, string, bgcolor, fcolor, 1);
	write_line(10, string, bgcolor, fcolor, 1);
	write_line(11, string, bgcolor, fcolor, 1);
};
int display_task(SensorData weather, DayCnt day_cnt, time_t utc, datetime_t dt){
	display.update = display.update_weather || display.update_time;
	if (display.update){
		display_info_update(weather);
		display_time_update(utc, dt, day_cnt);
		display_brightness((day) ? dcfg.day_brightness: dcfg.night_brightness);
		display_fill_empty_lines();
		display.update = false;
	}
}
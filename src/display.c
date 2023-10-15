#include "display.h"
#ifndef TEST
#include "pico/stdlib.h"
#endif
#include <stdio.h>
#include "st7789.h"
uint16_t rcolor;
uint16_t gcolor;
bool day;

uint16_t FRMBUF[FRMHEIGHT * FRMWIDTH] = {0};
uint16_t LINE[FRMHEIGHT_S * FRMWIDTH] = {0};
const char* month_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
//const char* day_str[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* day_str[] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

DisplayConfig dcfg;
DisplayInfo display;
typedef union{

	uint16_t word;
	struct{

		uint16_t red: 5;
		uint16_t green: 6;
		uint16_t blue: 5;
	}bits;
}c565;


uint16_t color565(uint8_t r, uint8_t g, uint8_t b){

	c565 value = {.bits={.red=(r * 31 / 255),.green=(g * 63 / 255),.blue=(b * 31 / 255)}};
	return value.word;
	//return (((g & 0x1c) << 3) + (b & 0xf8 ) << 5) + (((r & 0xf8)) + ((g & 0xe0) >> 5));
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

int fill_display(uint16_t color){
	int cnt = FRMHEIGHT / FRMHEIGHT_S;	
	int pixel_cnt = FRMHEIGHT * FRMWIDTH;

	for(int j = 0; j < FRMHEIGHT_S * FRMWIDTH; j++){
		LINE[j] = color;
	}


	for(int i = 0; i < cnt; i++){
		lcd_set_raset(i * FRMHEIGHT_S, (i + 1) * FRMHEIGHT_S - 1);
		lcd_set_caset(0, FRMWIDTH - 1);
		write_buffer(RAMWR, (uint8_t*) LINE, sizeof(LINE));
	}
}

int fill_display_gradient(){
	uint8_t r = 0;
	uint8_t b = 0;
	uint8_t g = 0;
	for(int j = 0; j < FRMHEIGHT * FRMWIDTH; j++){
		// blue = j % FRMWIDTH;
		// lb = (((red & 0xf8)) + ((green & 0xe0) >> 5));
		// hb = (((green & 0x1c) << 3) + (blue >> 3 ));
		//FRMBUF[j] = (hb << 8) + lb;

		FRMBUF[j] = (((g & 0x1c) << 3) + (b & 0xf8 ) << 5) +
						(((r & 0xf8)) + ((g & 0xe0) >> 5));
		r++; b++; g++;
	}
	return write_buffer(RAMWR, (uint8_t*) FRMBUF, sizeof(FRMBUF));
}

static void render_font(uint16_t* img, int k, int x , int y , uint16_t color, int ds){
    int offset = 4 + k * 96;
    const int h = 32;
    const int w = 24;
	uint16_t rcolor = color565(135, 43, 43);
	color = (k == 94) ? rcolor : color;
    unsigned char temp;
	int row_s = 0;
	int col_s = 0;
    for(int row=y; row < y + h; row++){
        for(int col=0; col < w / 8; col++){
            temp = Ubuntu[offset + ((row  - y) * 3) + (col)];

			for (int j = 0; j < 8; j++){
				if ((temp >> j) & 0x01){
					// img[(row * 240) + col * 8 + (7 - j) + x] = color;
					row_s = row / ds;
					col_s = (col * 8 + (7 - j)) / ds; 
					img[(row_s * FRMWIDTH) + col_s + x] = color;
				}
			}
		}
	}
}

int update_display(){
	return write_buffer(RAMWR, (uint8_t*) FRMBUF, sizeof(FRMBUF));
}

int write_string(uint16_t x, uint16_t y, char* string, uint16_t fcolor, int ds){
	int idx = 0;
	int colw = 24 / ds;
	int roww = 32 / ds;
	while(*string != '\0'){
		render_font(FRMBUF, (*string++) - 32, colw * idx + x, y, fcolor, ds);
		if(idx++ > (9 * ds)){break;}
	}
	return 0;
}

int write_line(uint16_t line_num, char* string,uint16_t bgcolor, uint16_t fcolor, int ds){
	int idx = 0;
	int colw = 24 / ds;
	int roww = 32 / ds;

	for(int j = 0; j < FRMHEIGHT_S * FRMWIDTH; j++){LINE[j] = bgcolor;}
	while(*string != '\0'){
		render_font(LINE, (*string++) - 32, colw * idx , 0, fcolor, ds);
		if(idx++ > (9 * ds)){break;}
	}

	lcd_set_raset(line_num * FRMHEIGHT_S, (line_num + 1) * FRMHEIGHT_S - 1);
	lcd_set_caset(0, FRMWIDTH - 1);
	write_buffer(RAMWR, (uint8_t*) LINE, sizeof(LINE));
	return 0;
}


#ifdef TEST
extern uint8_t img[FRMWIDTH * FRMHEIGHT * 3]; 
int display_brightness(int value){return 0;}

uint16_t line_start = 0;
uint16_t line_end = 0;
uint16_t col_start = 0;
uint16_t col_end = 0;


void icolor565(uint16_t color, uint8_t* colors){
	c565 value = {.word=color};
	colors[0] = value.bits.red << 3;
	colors[1] = value.bits.green << 2;
	colors[2] = value.bits.blue << 3;
}
static int write_buffer(uint8_t reg, uint8_t* buffer, int len){
	c565 pixel;
	uint16_t* pixel_ptr = (uint16_t*)buffer;
	int l_cnt = line_end - line_start;
	int c_cnt = col_end - col_start;
	uint8_t colors[3];
	printf("%d %d %d %d \n", line_start, line_end, col_start, col_end);
	for(int i = 0; i < l_cnt; i++){
		for(int j = 0; j < c_cnt; j++){
			pixel.word = pixel_ptr[i * FRMWIDTH + j];
			icolor565(pixel.word, &colors[0]);
			img[(i + line_start) * (FRMWIDTH * 3) + ((j + col_start) * 3) + 0 ] = colors[0];
			img[(i + line_start) * (FRMWIDTH * 3) + ((j + col_start) * 3) + 1 ] = colors[1];
			img[(i + line_start) * (FRMWIDTH * 3) + ((j + col_start) * 3) + 2 ] = colors[2];
		}
	}
	
	return 0;
}
int lcd_set_caset(uint16_t caset_xs, uint16_t caset_xe){
	col_start = caset_xs;
	col_end = caset_xe;
	return 0;
}
int lcd_set_raset(uint16_t raset_ys, uint16_t raset_ye){
	line_start = raset_ys;
	line_end = raset_ys + 40;
	return 0;
}

#endif
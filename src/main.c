#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#include "scd4x.h"
#include "st7789.h"
#include "ntp_client.h"
#include "peripherals.h"


#define TIME_START 1673823600

const char* month_str[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char* day_str[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
typedef struct SensorData{
	uint16_t co2_raw;
	uint16_t temperature_raw;
	uint16_t humidity_raw;
	float temperature;
	float humidity;
} SensorData;
SensorData weather;


typedef struct DisplayConfig{
	uint16_t day_background;
	uint16_t day_fontcolor;

	uint16_t day_brightness;
	uint16_t night_brightness;
	uint16_t night_background;
	uint16_t night_fontcolor;
} DisplayConfig;

DisplayConfig dcfg;

typedef struct DayCnt{
	int week;
	int day;
	time_t start;
}DayCnt;

DayCnt day_cnt={
	.day = 0,
	.week = 0,
	.start = 1673740800
};

uint16_t rcolor;
uint16_t gcolor;
bool rtc_time_updated = false;

int init_spi(){
	spi_init(spi_default, 24 * 1000 * 1000);
//	gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
	gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
	// Chip select is active-low, so we'll initialise it to a driven-high state
	gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
	gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
	spi_set_format(spi_default, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	return 0;
}

int init_peripherals(){
	stdio_init_all();
	init_spi();
	init_i2c(&i2c_moudle);
	init_st7789();
	rcolor = color565(0xe6,0x39, 0x46);
	gcolor = color565(34, 179, 34);
	dcfg.day_background=color565(220, 220, 220);
	dcfg.day_fontcolor=color565(127, 127, 127);
	dcfg.night_background=color565(0,0,0);
	dcfg.night_fontcolor=color565(127, 127, 127);
	dcfg.day_brightness=0xf000;
	dcfg.night_brightness=0x100;
	return 0;
}

int init_sensors(){
	uint16_t alt = 0;
	uint32_t serial;
	uint16_t status;
	int i = 0;
	stop_measurements();
	get_serial_number(&serial);
	get_sensor_altitude(&alt);
	set_sensor_altitude(562);
	get_sensor_altitude(&alt);
	start_measurements();
}

// Start on Friday 5th of June 2020 15:45:00
datetime_t t = {
		.year  = 2020,
		.month = 06,
		.day   = 05,
		.dotw  = 5, // 0 is Sunday, so 5 is Friday
		.hour  = 15,
		.min   = 45,
		.sec   = 00
};

int counter_task(){
	time_t _utc_time;
	struct tm utc ={.tm_year=t.year - 1900, .tm_mon=t.month -1, .tm_mday=t.day, .tm_wday=t.dotw, .tm_hour=t.hour, .tm_min=t.min, .tm_sec=t.sec, .tm_isdst=0};
	_utc_time = mktime(&utc);
	time_t duration = (_utc_time - day_cnt.start) / 3600 / 24;
	day_cnt.week = duration / 7;
	day_cnt.day = duration % 7;
}



bool day = false;
int rtc_task(){
	if(time_updated){
		struct tm *utc = gmtime(&utc_time);
		t.year = utc->tm_year + 1900;
		t.month = utc->tm_mon + 1;
		t.day = utc->tm_mday;
		t.dotw = utc->tm_wday;
		t.hour = utc->tm_hour;
		t.min = utc->tm_min;
		t.sec = utc->tm_sec;
		time_updated = false;
		rtc_set_datetime(&t);
	}

	rtc_get_datetime(&t);
	day =  ((t.hour > 8) && (t.hour < 20)) ? true: false;
	display.update_time = true;
	return 0;
}
int sensor_task(){
	int status = 0;
	if (get_status_ready())
	{
		status = read_measurements(&weather.co2_raw, &weather.temperature_raw, &weather.humidity_raw);
		display.update_weather = true;
	}
	if(!status){
		weather.temperature = -45 + (float)(175 * weather.temperature_raw) / (65536.0);
		weather.humidity = (float)(100 * weather.humidity_raw) / (65536.0);
	}
}
static int display_time_update(){
	char string[20];
	uint16_t fcolor = (day) ? dcfg.day_fontcolor: dcfg.night_fontcolor;

	struct tm *utc = gmtime(&utc_time);
	sprintf(string, "  %s %02d %s", day_str[t.dotw], t.day , month_str[t.month - 1]);
	write_string(display.ML, 6 , string, fcolor, 2);
	sprintf(string, "      %02d:%02d:%02d", t.hour , t.min, t.sec);
	write_string(display.ML, 3 , string, fcolor, 2);
	rtc_time_updated = true;

	sprintf(string, " %02dW %dD ~", day_cnt.week, day_cnt.day);
	write_string(display.ML, 7 , string, fcolor, 1);
	display.update_time = false;
}
static int display_info_update(){
	char string[20];
	uint16_t fcolor = (day) ? dcfg.day_fontcolor: dcfg.night_fontcolor;
	printf("CO2: %d\n", weather.co2_raw);
	sprintf(string, "CO2: %d", weather.co2_raw);
	write_string(display.ML, 3 , string, (weather.co2_raw < 1200)? gcolor: rcolor, 1);
	sprintf(string, "TMP: %2.1f", weather.temperature);
	write_string(display.ML, 4 , string, fcolor, 1);
	sprintf(string, "RH: %2.1f%%", weather.humidity);
	write_string(display.ML, 5 , string, fcolor, 1);
	display.update_weather = false;
}
static int display_task(){
	display.update = display.update_weather || display.update_time;
	if (display.update){
		fill_display((day) ? dcfg.day_background: dcfg.night_background);
		display_info_update();
		display_time_update();

		display_brightness((day) ? dcfg.day_brightness: dcfg.night_brightness);

		update_display();
		display.update = false;
	}
}

int main(){
	init_peripherals();
	init_display();
	init_sensors();
	init_wifi();
	init_rtc();
	fill_display(color565(0, 0, 0));
	ntp_task();
	while (true){
		sensor_task();
		display_task();
		counter_task();
		rtc_task();
		sleep_ms(1000);
	}
	return 0;
}

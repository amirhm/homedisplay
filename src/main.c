#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "scd4x.h"
#include "ntp_client.h"
#include "st7789.h"
#include "peripherals.h"
#include "display.h"


SensorData weather;
DayCnt day_cnt={
	.day = 0,
	.week = 0,
	.start = 1673740800
};

uint16_t rcolor;
uint16_t gcolor;


int init_sensors(){
	uint16_t alt = 0;
	uint32_t serial;
	uint16_t status;
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

int main(){
	init_peripherals();
	init_disp();
	init_display();
	init_sensors();
	init_wifi();
	init_rtc();
	ntp_task();
	while (true){
		sensor_task();
		display_task(weather, day_cnt, utc_time, t);
		counter_task();
		rtc_task();
		sleep_ms(1000);
	}
	return 0;
}

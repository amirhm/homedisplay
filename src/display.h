#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "pico/util/datetime.h"

#include <time.h>
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
#endif
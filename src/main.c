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


#define TIME_START 1673823600

#define WIFI_SSID "Salt_2GHz_28DB5E_2.4GHz_2.4GHz"
#define WIFI_PASSWORD "aMirhm2153"
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

typedef struct DayCnt{
	int week;
	int day;
}DayCnt;

DayCnt day_cnt;

bool rtc_time_updated;
int init_i2c(){
	i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
	return 0;
}


int init_spi(){
	spi_init(spi_default, 5 * 1000 * 1000);
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
	init_gpio();
	init_spi();
	init_i2c();
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

int counter_task(){
	time_t duration = (utc_time - 1673740800) / 3600 / 24;
		day_cnt.week = duration / 7;
		day_cnt.day = duration % 7;
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

int init_rtc(){
	// Start the RTC
	rtc_init();
	rtc_time_updated = false;
}

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
	uint16_t fcolor = color565(127, 127, 127);
	uint16_t rcolor = color565(135, 43, 43);
	uint16_t gcolor = color565(34, 179, 34);
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
	uint16_t fcolor = color565(127, 127, 127);
	uint16_t rcolor = color565(135, 43, 43);
	uint16_t gcolor = color565(34, 179, 34);
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
		fill_display(color565(0, 0, 0));
		display_info_update();
		display_time_update();

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

	while (true){
		sensor_task();
		display_task();
		ntp_task();
		counter_task();
		rtc_task();
		// printf("%02d/%02d/%04d\n", utc_time->tm_mday, utc_time->tm_mon + 1, utc_time->tm_year + 1900);
		// printf("%02d:%02d:%02d\n", utc_time->tm_hour, utc_time->tm_min, utc_time->tm_sec);
		sleep_ms(100);
}
	printf("Done.\n");
	return 0;
}
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "scd4x.h"
#include "st7789.h"


typedef struct SensorData{
	uint16_t co2_raw;
	uint16_t temperature_raw;
	uint16_t humidity_raw;
	float temperature
	float humidity;
} SensorData;
SensorData weather;


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


int sensor_task(){
	int status = 0;
	if (get_status_ready())
	{
		status = read_measurements(&weather.co2_raw, &weather.temperature_raw, &weather.humidity_raw);
		display.updated = true;
		display.update_weather = true;
	}
	if(!status){
		temperature = -45 + (float)(175 * weather.temperature_raw) / (65536.0);
		rhumid = (float)(100 * weather.humidity_raw) / (65536.0);
		sleep_ms(10);
	}
}
static int display_info_update(){
	write_number(display.ML, display.LineHeight, weather.co2);
	write_number(display.ML, display.LineHeight * 2, (uint16_t) weather.temperature);
	write_number(display.ML, display.LineHeight * 3, (uint16_t) weather.humidity);
	display.update_weather = false;
}
static int display_task(){
	if (display.updated){
		fill_display(color565(0,0,0));
	}
	if (display.update_weather){
		display_info_update();
	}
	if (display.updated){
		update_display();
		display.update = false;
	}
}

int main(){
	init_peripherals();
	init_display();
	init_sensors();

	fill_display(color565(0, 0, 0));

	while (true){
		sensor_task();
		display_task();
	}
	printf("Done.\n");
	return 0;
}
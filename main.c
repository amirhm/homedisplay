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


#if 0
int blk_init(){
	// Tell GPIO blk they are allocated to the PWM
	gpio_set_function(blk, GPIO_FUNC_PWM);

	// Find out which PWM slice is connected to GPIO 0 (it's slice 0)
	uint slice_num = pwm_gpio_to_slice_num(blk);
	pwm_config config = pwm_get_default_config();
	// Set divider, reduces counter clock to sysclock/this value
	pwm_config_set_clkdiv(&config, 4.f);
	// Load the configuration into our PWM slice, and set it running.
	pwm_init(slice_num, &config, true);
	return slice_num;
}

void blk_set_value(int slice_num, int val){
	// Set period of 4 cycles (0 to 3 inclusive)
	pwm_set_wrap(slice_num, 3);
	// Set channel A output high for one cycle before dropping
	pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
	// Set initial B output high for three cycles before dropping
	pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
	// Set the PWM running
	pwm_set_enabled(slice_num, true);
}
#endif



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

int main(){
	stdio_init_all();
	init_gpio();
	// int slice_n = blk_init();
	init_spi();
	init_i2c();

	init_display();
	stop_measurements();
	uint16_t alt = 0;
	uint32_t serial;
	get_serial_number(&serial);
	get_sensor_altitude(&alt);
	set_sensor_altitude(562);
	get_sensor_altitude(&alt);

	uint16_t co2, temp, rh;
	float temperature, rhumid;
	uint16_t status;

	int i = 0;
	start_measurements();
	fill_display_gradient();
	sleep_ms(1000);
	fill_display(color565(0, 0, 0));

	while (true){
		if (get_status_ready() )
		{
			read_measurements(&co2, &temp, &rh);
			temperature = -45 + (float)(175 * temp) / (65536.0);
			rhumid = (float)(100 * temp) / (65536.0);
			printf("co2 %d temp %f  rh %f\n", co2, temperature, rhumid);
			sleep_ms(10);
			i++;
			fill_display(color565(0,0,0));

			// write_character(100, 150, (i % 10));
			write_number(10, 50, co2);
			write_number(10, 150, (uint16_t) temperature);
			write_number(10, 250, (uint16_t) rhumid);
			update_display();
		}
	}

	printf("Done.\n");
	return 0;
}
#if 0
int mains() {
	stdio_init_all();
	init_gpio();
	// int slice_n = blk_init();
	init_spi();
	init_i2c();
}
#endif
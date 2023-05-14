#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
const int rst = 21;
const int dc = 20;
const int blk = 15;

#define FRMWIDTH 240
#define FRMHEIGHT 300

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

void init_gpio(){

	gpio_init(dc);
	gpio_init(rst);
	gpio_init(blk);

	gpio_set_dir(rst, GPIO_OUT);
	gpio_set_dir(dc, GPIO_OUT);
	gpio_set_dir(blk, GPIO_OUT);

	gpio_put(rst, 1);
	gpio_put(dc, 0);
	gpio_put(blk, 1);
}

int init_spi(){

	spi_init(spi_default, 5 * 1000 * 1000);
//	spi_set_format(spi_default, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
//	gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
	gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
	// Make the SPI pins available to picotool
//	bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

	// Chip select is active-low, so we'll initialise it to a driven-high state
	gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
	gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
	// Make the CS pin available to picotool
//	bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));
	spi_set_format(spi_default, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

}

static void write_register(uint8_t reg, uint8_t data) {
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(1);
	spi_write_blocking(spi_default, &reg, 1);
	sleep_us(1);
	gpio_put(dc, 1);  // Active low
	sleep_us(1);
	spi_write_blocking(spi_default, &data, 1);
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(1);
}

static void write_buffer(uint8_t reg, uint8_t* buffer, int len) {
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(10);
	spi_write_blocking(spi_default, &reg, 1);
	sleep_us(10);

	gpio_put(dc, 1);  // Active low
	sleep_us(10);
	spi_write_blocking(spi_default, buffer, len);
	sleep_us(10);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(10);
}	

static void send_cmd(uint8_t cmd) {
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(1);
	spi_write_blocking(spi_default, &cmd, 1);
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(1);
}


typedef enum CMD{
	SWRESET     = 0x01,
	INVON = 0x21,
	INVOFF = 0x20,
	SLPOUT = 0x11,
	DISPON = 0x29,
	CASET = 0x2a,
	RASET = 0x2b,
	 TEON        = 0x35,
	MADCTL = 0x36,
	COLMOD = 0x3A,
	RAMWR = 0x2c
} CMD;


void lcd_reset(){

    gpio_put(rst, 1);
    sleep_ms(100);
    gpio_put(rst, 0);
    sleep_ms(100);
    gpio_put(rst, 1);
    sleep_ms(100);
}

uint16_t FRMBUF[FRMHEIGHT * FRMWIDTH] = {0};


int main() {
	stdio_init_all();
	init_gpio();
	// int slice_n = blk_init();
	init_spi();
	lcd_reset();
	send_cmd(SWRESET);
	sleep_ms(100);
	send_cmd(TEON);
	sleep_ms(1);
	write_register(COLMOD, 0x05);
	sleep_ms(1);
	send_cmd(INVON);
	send_cmd(SLPOUT);
	send_cmd(DISPON);
	uint16_t caset_xs = 0;
	uint16_t caset_xe = FRMWIDTH;
	uint8_t caset[4] = {
		(caset_xs & 0xff00) >> 8,
		caset_xs & 0x00ff,
		(caset_xe & 0xff00) >> 8,
		caset_xe & 0x00ff
	};

	uint16_t raset_ys = 0;
	uint16_t raset_ye = FRMHEIGHT;
	uint8_t raset[4] = {
		(raset_ys & 0xff00) >> 8,
		raset_ys & 0x00ff,
		(raset_ye & 0xff00) >> 8,
		raset_ye & 0x00ff
	};
	uint8_t madctl = 0;

	write_buffer(CASET, caset, 4);
	write_buffer(RASET, raset, 4);
	write_register(MADCTL, 0);
	uint8_t red = 0;
	uint8_t green = 0;
	uint8_t blue = 0;
	while(true){
		//blk_set_value(slice_n, val);
		// pwm_set_gpio_level(blk, 0x7fff);
		//uint8_t lb = 0;
		//uint8_t hb = 0;
		for(int j = 0; j < FRMHEIGHT * FRMWIDTH; j++){
			blue = j % FRMWIDTH;
			//lb = (((red & 0xf8)) + ((green & 0xe0) >> 5));
			//hb = (((green & 0x1c) << 3) + (blue >> 3 ));
			//FRMBUF[j] = (hb << 8) + lb;

			FRMBUF[j] = (((green & 0x1c) << 3) + (blue & 0xf8 ) << 5) +
						(((red & 0xf8)) + ((green & 0xe0) >> 5));
		}
		write_buffer(RAMWR, (uint8_t*) FRMBUF, sizeof(FRMBUF)) ;
		sleep_ms(1);

	}
}

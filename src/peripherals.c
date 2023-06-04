#include "peripherals.h"


I2C_MODULE i2c_moudle = {
    .inst=i2c_default, 
    .sda_pin=PICO_DEFAULT_I2C_SDA_PIN, // 24 
    .scl_pin=PICO_DEFAULT_I2C_SCL_PIN, // 25
    .clk_freq_KHz=100
};

SPI_MODULE spi_module = {
    .inst=spi_default,
    .clk_freq_MHz = 24,
    .csn_pin=PICO_DEFAULT_SPI_CSN_PIN,
    .tx_pin=PICO_DEFAULT_SPI_TX_PIN,
    .rx_pin=PICO_DEFAULT_SPI_RX_PIN,   
    .sck_pin=PICO_DEFAULT_SPI_SCK_PIN
};


ST7789 st7789 = {
	.spi_module=&spi_module,
    .blk = 15,
    .dc = 20,
    .reset = 21
};

int init_i2c(I2C_MODULE* instance){
	i2c_init(instance->inst, instance->clk_freq_KHz * 1000);
    gpio_set_function(instance->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(instance->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(instance->scl_pin);
    gpio_pull_up(instance->sda_pin);
	return 0;
}

int init_spi(SPI_MODULE* instance){
	spi_init(instance->inst, 24 * 1000000);
//	gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
	gpio_set_function(instance->tx_pin, GPIO_FUNC_SPI);
	gpio_set_function(instance->sck_pin, GPIO_FUNC_SPI);

	// Chip select is active-low, so we'll initialise it to a driven-high state
	gpio_init(instance->csn_pin);
	gpio_set_dir(instance->csn_pin, GPIO_OUT);
	gpio_put(instance->csn_pin, 1);
	spi_set_format(instance->inst, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	return 0;
}


int init_rtc(){
	// Start the RTC
	rtc_init();
}

int init_st7789(ST7789* inst){
	gpio_set_function(inst->blk, GPIO_FUNC_PWM);


    uint sliceNum = pwm_gpio_to_slice_num(inst->blk);
    pwm_config config = pwm_get_default_config();
    pwm_init(sliceNum, &config, true);

	gpio_init(inst->dc);
	gpio_init(inst->reset);

	gpio_set_dir(inst->reset, GPIO_OUT);
	gpio_set_dir(inst->dc, GPIO_OUT);

	gpio_put(inst->reset, 1);
	gpio_put(inst->dc, 0);
	pwm_set_gpio_level(inst->blk, 0x7fff);
}

int init_peripherals(){
	stdio_init_all();
	init_spi(&spi_module);
	init_i2c(&i2c_moudle);
	init_st7789(&st7789);
	return 0;
}

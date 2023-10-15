#include "st7789.h"
const int rst = 21;
const int dc = 20;
const int blk = 15;


int display_brightness(int value){
	pwm_set_gpio_level(blk, value);
	return 0;
}

static void write_register(uint8_t reg, uint8_t data) {
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

int write_buffer(uint8_t reg, uint8_t* buffer, int len) {
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);
	gpio_put(dc, 0);
	sleep_us(10);
	spi_write_blocking(spi_default, &reg, 1);
	sleep_us(10);

	gpio_put(dc, 1);
	sleep_us(10);
	spi_write_blocking(spi_default, buffer, len);
	sleep_us(10);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
	gpio_put(dc, 0);
	sleep_us(10);
	return 0;
}

static void send_cmd(uint8_t cmd) {
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);  // Active low
	gpio_put(dc, 0);  // Active low
	sleep_us(1);
	spi_write_blocking(spi_default, &cmd, 1);
	sleep_us(1);
	gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
	gpio_put(dc, 0);
	sleep_us(1);
}

void lcd_reset(){
	gpio_put(rst, 1); sleep_ms(100);
	gpio_put(rst, 0); sleep_ms(100);
	gpio_put(rst, 1); sleep_ms(100);
}


int lcd_set_caset(uint16_t caset_xs, uint16_t caset_xe)
{
    uint8_t caset[4] = {
        (caset_xs & 0xff00) >> 8, caset_xs & 0x00ff,
        (caset_xe & 0xff00) >> 8, caset_xe & 0x00ff
        };
    return write_buffer(CASET, caset, 4);
}

int lcd_set_raset(uint16_t raset_ys, uint16_t raset_ye){
    uint8_t raset[4] = {
        (raset_ys & 0xff00) >> 8, raset_ys & 0x00ff,
        (raset_ye & 0xff00) >> 8, raset_ye & 0x00ff
        };
    return write_buffer(RASET, raset, 4);
}

int init_display(){
	lcd_reset();
	send_cmd(SWRESET);
	sleep_ms(100);
	send_cmd(SLPOUT);
	sleep_ms(100);

	send_cmd(DISPOFF);

	write_register(0xF0, 0xc3); //Command Set control                                 
								//Enable extension command 2 partI
	
	write_register(0xF0, 0x96); //Command Set control                                 
								//Enable extension command 2 partII
	
	write_register(0x36, 0x48); //Memory Data Access Control MX, MY, RGB mode                                   
						//X-Mirror, Top-Left to right-Buttom, RGB  



	write_register(COLMOD, 0x55);//Interface Pixel Format   //Control interface color format set to 16                                 
	write_register(0xB4, 0x01); //Column inversion //1-dot inversion 
    //1-dot inversion




	uint8_t data[] = {
		0x80,    //Bypass
		0x02,    //Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
		0x3B    //LCD Drive Line=8*(59+1)
	};

	write_buffer(0xB6, data, sizeof(data)); //Display Function Control

	uint8_t cntrl_adjuts[]={
		0x40,
		0x8A,	
		0x00,
		0x00,
		0x29,    //Source eqaulizing period time= 22.5 us
		0x19,    //Timing for "Gate start"=25 (Tclk)
		0xA5,    //Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
		0x33
	};
	write_buffer(0xE8, cntrl_adjuts, sizeof cntrl_adjuts); //Display Output Ctrl Adjust
	
	write_register(0xC1, 0x06); //Power control2                          
								//VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)
	 
	write_register(0xC2, 0xA7); //Power control 3                                      
								//Source driving current level=low, Gamma driving current level=High
	 
	write_register (0xC5, 0x18); 	//VCOM Control
									//VCOM=0.9
	sleep_ms(120);
	
	//ST7796 Gamma Sequence
	uint8_t buffer0[] = {
		0xF0,	0x09,	0x0b,	0x06,	0x04,	0x15,	0x2F, 
		0x54,	0x42,	0x3C,	0x17,	0x14,	0x18,	0x1B
		};	 
	write_buffer(0xE0, buffer0, sizeof(buffer0));

	uint8_t buffer1[] = {
		0xE0,	0x09,	0x0B,	0x06,	0x04,	0x03,	0x2B,
		0x43,	0x42,	0x3B,	0x16,	0x14,	0x17,	0x1B
	};
	
	write_buffer(0xE1, buffer0, sizeof(buffer1)); //Gamma"-"                                             
	sleep_ms(120);
	
	write_register(0xF0, 0x3C); //Command Set control                                 
								//Disable extension command 2 partI

	write_register(0xF0, 0x69); //Command Set control                                 
								//Disable extension command 2 partII


	// lcd_set_caset(0, FRMWIDTH - 1);
	// lcd_set_raset(0, FRMHEIGHT - 1);
	// write_register(MADCTL, 0x18);

	send_cmd(SLPOUT);
	sleep_ms(150);
	send_cmd(DISPON);
}


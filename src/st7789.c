#include "st7789.h"
#include "peripherals.h"
#include "Ubuntu.h"
const int rst = 21;
const int dc = 20;
const int blk = 15;


#define FRMHEIGHT_S  40

uint16_t FRMBUF[FRMHEIGHT * FRMWIDTH] = {0};
uint16_t LINE[FRMHEIGHT_S * FRMWIDTH] = {0};

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

static int write_buffer(uint8_t reg, uint8_t* buffer, int len) {
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


	lcd_set_caset(0, FRMWIDTH - 1);
	lcd_set_raset(0, FRMHEIGHT - 1);
	// write_register(MADCTL, 0x18);

	send_cmd(SLPOUT);
	sleep_ms(150);
	send_cmd(DISPON);
}


static uint16_t color565(uint8_t r, uint8_t g, uint8_t b){
	return (((g & 0x1c) << 3) + (b & 0xf8 ) << 5) + (((r & 0xf8)) + ((g & 0xe0) >> 5));
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
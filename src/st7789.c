#include "st7789.h"
#include "Ubuntu.h"

const int rst = 21;
const int dc = 20;
const int blk = 15;


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


typedef enum CMD{
	SWRESET     = 0x01,
	INVON = 0x21,
	INVOFF = 0x20,
	SLPOUT = 0x11,
	DISPON = 0x29,
	CASET = 0x2a,
	RASET = 0x2b,
	TEON = 0x35,
	MADCTL = 0x36,
	COLMOD = 0x3A,
	RAMWR = 0x2c
} CMD;

void lcd_reset(){
	gpio_put(rst, 1); sleep_ms(100);
	gpio_put(rst, 0); sleep_ms(100);
	gpio_put(rst, 1); sleep_ms(100);
}

uint16_t FRMBUF[FRMHEIGHT * FRMWIDTH] = {0};

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
	send_cmd(TEON);
	sleep_ms(1);
	//set Color mode
	write_register(COLMOD, 0x05);
	sleep_ms(1);
	send_cmd(INVON);
	send_cmd(SLPOUT);
	send_cmd(DISPON);
	lcd_set_caset(0, FRMWIDTH);
	lcd_set_raset(0, FRMHEIGHT);
	write_register(MADCTL, 0);
	display.ML = 10;
	display.LineHeight = 32;
}


uint16_t color565(uint8_t r, uint8_t g, uint8_t b){
	return (((g & 0x1c) << 3) + (b & 0xf8 ) << 5) + (((r & 0xf8)) + ((g & 0xe0) >> 5));
}
int fill_display(uint16_t color){
	for(int j = 0; j < FRMHEIGHT * FRMWIDTH; j++){
		FRMBUF[j] = color;
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

extern const unsigned char Ubuntu[9124];
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
					img[(row_s * 240) + col_s + x] = color;
				}
			}
		}
	}
}
#if 0
int write_character(uint16_t x, uint16_t y, uint16_t number)
{
	uint8_t w = 25;
	uint8_t h = 45;
	uint32_t idx = 0;
	uint16_t color = 0;
	for(int j = 0; j < w * h; j++){
		//xf = j / 25;
		//yf = j % 25;
		idx = (y + (j / 25)) * FRMWIDTH + (x + (j % 25));
		color = color565(255 - digits[number][j], 255 - digits[number][j], 255 - digits[number][j]);
		FRMBUF[idx] = color;
	}
}
#endif
int update_display(){
	return write_buffer(RAMWR, (uint8_t*) FRMBUF, sizeof(FRMBUF));
}

int write_number(uint16_t x, uint16_t y, uint16_t number){
	int cnt = 0;
	int dig = 1;
	uint8_t w0 = 5 * 25;
	while (number > 0){
		dig = number % 10;
		number /= 10;
		write_character(x + w0, y, dig);
		w0 -= 25;
	}
	return 0;

}
int write_string(uint16_t x, uint16_t y, char* string, uint16_t fcolor, int ds){
	int idx = 0;
	int colw = 24 / ds;
	int roww = 32 / ds;
	while(*string != '\0'){
		render_font(FRMBUF, (*string++) - 32, colw * idx + x, roww * y, fcolor, ds);
		if(idx++ > (9 * ds)){break;}
	}
	return 0;
}
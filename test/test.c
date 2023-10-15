
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "display.h"
#include "st7789.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define FRMWIDTH 480
#define FRMHEIGHT 320
#define FRMHEIGHT_S  40


uint8_t img[FRMWIDTH * FRMHEIGHT * 3]; 
int main(void) {

    const int width = FRMWIDTH, height = FRMHEIGHT;
    const int frmsize = FRMWIDTH * FRMHEIGHT;


    memset(img, 0xff, sizeof img);

    for (int i = 0; i <height; i++){
        for (int j = 0; j <width; j++){
           img[(i * (width * 3) ) + (j * 3) + 0] = 0x00; 
           img[(i * (width * 3) ) + (j * 3) + 1] = 0x00; 
           img[(i * (width * 3) ) + (j * 3) + 2] = 0x00; 
        }
    }

	char string[20];
	uint16_t fcolor = color565(0xe6,0x39, 0x46);
	uint16_t gcolor = color565(34, 179, 34);
	uint16_t bgcolor = color565(0xFF,0xFF, 0xFF);
	printf("CO2: %d\n", 105);
    //fill_display(bgcolor);
	sprintf(string, "CO2: %d", 505);
	write_line(2, string, bgcolor, fcolor, 5);
	write_line(3, string, bgcolor, gcolor, 1);


    stbi_write_png("screen.png", width, height, 3, img, width * 3);

    //stbi_image_free(img); 
}
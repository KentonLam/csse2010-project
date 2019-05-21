/*
 * display.c
 *
 * Created: 21/05/2019 6:15:54 PM
 *  Author: Kenton
 */ 

#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "pixel_colour.h"
#include "display.h"
#include "ledmatrix.h"

#define LED_MATRIX_POSN_FROM_XY(gameX, gameY)		(gameY) , (7-(gameX))

uint32_t curState[16];
uint32_t newState[16];

char terminalBuffer[255];

uint8_t readingIntoFrame = 0;

/*uint8_t stateBitMask;*/

void reset_frame() {
	memset(curState, 0, sizeof(curState));	
	memset(newState, 0, sizeof(newState));	
}

void new_frame() {
// 	if (readingIntoFrame) {
// 		printf_P(PSTR("FAULT: new_frame called while reading new frame!"));
// 		while (1){}
// 	}
	readingIntoFrame = 1;
	/*stateBitMask = (~bitsToClear) & 0xf;*/

}

void set_pixel(uint8_t x, uint8_t y, uint8_t colour) {
	if (!readingIntoFrame) {
		printf_P(PSTR("FAULT: set_pixel called without frame!"));
		while (1){}
	}
	uint32_t val;
	switch (colour) {
		case COLOUR_BLACK:
		val = CODE_BLACK;
		break;
		case COLOUR_GREEN:
		val = CODE_GREEN;
		break;
		case COLOUR_RED:
		val = CODE_RED;
		break;
		case COLOUR_YELLOW:
		val = CODE_YELLOW;
		break;
		default:
		val = 0;
		break;
	}
	
	newState[y] |= (val) << (4*x);	
}

// https://stackoverflow.com/a/53175/2734389
uint8_t hob (uint8_t num)
{
	if (!num)
		return 0;
	uint8_t ret = 1;
	while (num >>= 1)
		ret <<= 1;
	return ret;
}

void draw_pixel(uint8_t x, uint8_t y, uint8_t colour) {
	ledmatrix_update_pixel(LED_MATRIX_POSN_FROM_XY(x, y), colour);
}

void draw_frame() {
	if (!readingIntoFrame) {
		printf_P(PSTR("FAULT: draw_frame called without frame!"));
		while (1){}
	}
	readingIntoFrame = 1;
	for (uint8_t y = 0; y < 16; y++) {
		/*printf("y=%d, %x\n", y, curState[y]);*/
		for (uint8_t x = 0; x < 8; x++) {
			
			uint32_t cur_bits = (curState[y]>>(4*x)) & 0xf;
			/*printf("  x=%d, %x\n", x, cur_bits);*/
			uint32_t new_bits = ((newState[y]>>(4*x))&0xf);
			if (new_bits == 0) {
				/*printf("Null %d,%d", x, y);*/
				 continue; // no colour requests received.
			}
			uint8_t new_code = hob(new_bits);
			
			if (new_code == hob(cur_bits)) {
				/*printf("Same %d,%d", x, y);*/
				continue; // same colour already displayed.
			}
			/*printf("Drawing %d,%d", x, y);*/
			curState[y] = (curState[y] & ~(0xf<<(4*x))) | (new_code << (4*x) );
			uint8_t val;
			switch (new_code) {
				case CODE_BLACK:
				val = COLOUR_BLACK;
				break;
				case CODE_GREEN:
				val = COLOUR_GREEN;
				break;
				case CODE_RED:
				val = COLOUR_RED;
				break;
				case CODE_YELLOW:
				val = COLOUR_YELLOW;
				break;
				default:
				val = 0;
				break;
			}
			draw_pixel(x, y, val);
		}
	}
	memset(newState, 0, sizeof(newState));
		
}
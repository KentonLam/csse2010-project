/*
 * display.c
 *
 * Created: 21/05/2019 6:15:54 PM
 *  Author: Kenton
 */ 

#include <avr/io.h>
#include <string.h>
#include "pixel_colour.h"
#include "display.h"

#define LED_MATRIX_POSN_FROM_XY(gameX, gameY)		(gameY) , (7-(gameX))

uint32_t curState[16];
uint32_t newState[16];

char terminalBuffer[255];

uint8_t stateBitMask;


void new_frame(uint8_t bitsToClear) {
	stateBitMask = (~bitsToClear) & 0xf;
	memset(newState, 0, sizeof(newState));
}

void set_pixel(uint8_t x, uint8_t y, uint8_t colour) {
	uint8_t val;
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
	
	newState[y] |= val << (4*x);	
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
	
}


void draw_frame() {
	for (uint8_t y = 0; y < 16; y++) {
		for (uint8_t x = 0; x < 8; x++) {
			uint8_t cur_bits = (curState[y]>>(4*x)) & 0xf;
			uint8_t new_bits = ((newState[y]>>(4*x))&0xf);
			if (new_bits == 0) {
				 continue; // no colour requests received.
			}
			uint8_t colour_code = hob(new_bits);
			
			if (colour_code == hob(cur_bits)) {
				continue; // same colour already displayed.
			}
			
			curState[y] = (curState[y] & ~(0xf<<(4*x))) | (colour_code << (4*x) );
			uint8_t val;
			switch (colour_code) {
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
			ledmatrix_update_pixel(LED_MATRIX_POSN_FROM_XY(x, y), val);
		}
	}
		
}
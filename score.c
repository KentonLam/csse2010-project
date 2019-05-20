/*
 * score.c
 *
 * Written by Peter Sutton
 */

#include "score.h"
#include "terminalio.h"
#include <avr/io.h>
#include <stdio.h>

#define MAX_LIVES 4 
#define FLASH_PERIOD 100 
#define FLASH_ON 50

uint8_t seven_seg[10] = { 63,6,91,79,102,109,125,7,127,111};

int32_t score = -1;
uint8_t left = 0;
int32_t lives = -1;
uint8_t tick = 0;

static void print_score();
static void print_lives();

void init_score(void) {
	// initialise pinouts for ssd.
	DDRC = 0xff;
	PORTC = 0xff;
	//     seven seg CC  lives LEDs
	DDRD |= (1<<DDRD2) | 0b01111000;
	
	score = 0;
	lives = MAX_LIVES;
	print_score();
	print_lives();
}

void add_to_score(int16_t value) {
	if (score+value < 0) {
		score = 0;
	} else {
		score += value;
	}
	print_score();
}

int32_t get_score(void) {
	return score;
}

void print_score(void) {
	move_cursor(2, 2);
	printf("Score: %3lu", score);
}

void update_score_tick(void) {
	if (score != -1) {
		uint8_t digit = 0;
		if (left) {
			digit = (score/10)%10;
		} else {
			digit = score % 10;
		}	
		PORTD = (PORTD&~(1<<PORTD2)) | (left << PORTD2);
		if ( (lives == 0 && tick >= FLASH_ON) || (left && digit == 0) ) {
			PORTC = 0;
		} else {
			PORTC = seven_seg[digit];
		}
	}
	
	if (lives == 0) {
		uint8_t base = PORTD & ~(0b1111 << 3);
		PORTD = base | (  (tick >= FLASH_ON ? 0 : 0xf) << 3);
	}
	
	left ^= 1;
	if (tick++ > FLASH_PERIOD)
		tick = 0;
}

void change_lives(int8_t change) {
	lives += change; 
	if (lives < 0)
		lives = 0;	
	print_lives();
}

void print_lives() {
	/* LEDS
	2 3 4 5 
	G O R G
	*/
	uint8_t leds = 0;
	switch (lives) {
		case 4:
		leds = 0b1111;
		break;
		
		case 3:
		leds = 0b0111;
		break;
		
		case 2:
		leds = 0b0110;
		break;
		
		case 1:
		leds = 0b0010;
		break;
		
		case 0:
		leds = 0;
		break;	
	}
	uint8_t base = PORTD & ~(0b1111 << 3);
	PORTD = base | (leds << 3);	
}

int32_t get_lives() {
	return lives;
}
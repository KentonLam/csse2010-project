/*
 * score.c
 *
 * Written by Peter Sutton
 */

#include "score.h"
#include "terminalio.h"
#include <avr/io.h>
#include <stdio.h>

uint8_t seven_seg[10] = { 63,6,91,79,102,109,125,7,127,111};
	
uint32_t score = -1;
uint8_t right = 0;

static void print_score();

void init_score(void) {
	// initialise pinouts for ssd.
	DDRC = 0xff;
	PORTC = 0xff;
	DDRD |= 1<<DDRD2;
	
	score = 0;
	print_score();
}

void add_to_score(uint16_t value) {
	score += value;
	print_score();
}

uint32_t get_score(void) {
	return score;
}

void print_score(void) {
	move_cursor(0, 0);
	printf("Score: %3lu", score);
}

void update_score_ssd(void) {
	if (score == -1)
		return;
		
	uint8_t digit = 0;
	if (right) {
		digit = (score/10)%10;
	} else {
		digit = score % 10;
	}	
	PORTD = (PORTD&~(1<<PORTD2)) | (right << PORTD2);
	PORTC = seven_seg[digit];
	right ^= 1;
}
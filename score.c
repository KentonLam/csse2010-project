/*
 * score.c
 *
 * Written by Peter Sutton
 */

#include "score.h"
#include "terminalio.h"
#include <avr/io.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define MAX_LIVES 4 
#define FLASH_PERIOD 100 
#define FLASH_ON 50

uint8_t seven_seg[10] = { 63,6,91,79,102,109,125,7,127,111};

int32_t score = -1;
uint8_t left = 0;
int32_t lives = -1;
uint8_t tick = 0;

uint8_t score_x;
uint8_t score_y;

static void print_score();
static void print_lives();

void init_score(uint8_t x, uint8_t y) {
	score_x = x;
	score_y = y;
	
	// initialise pinouts for ssd.
	DDRC = 0xff;
	PORTC = 0xff;
	//     seven seg CC  lives LEDs
	DDRD |= (1<<DDRD2);
	DDRA |= 0xf0;
	
	
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
	fast_set_display_attribute(TERM_RESET);
	move_cursor(score_x, score_y);
	printf("Score:%6lu", score);
}

void update_score_tick(void) {
	if (score != -1) {
		uint8_t digit = 0;
		uint8_t dp = 0;
		uint16_t shifted_score = score;
		while (shifted_score >= 100) {
			shifted_score /= 10;
		}
		if (left) {
			digit = (shifted_score/10)%10;
			dp = score >= 1000;
		} else {
			digit = shifted_score % 10;
			dp = score >= 100;
		}	
		PORTD = (PORTD&~(1<<PORTD2)) | (left << PORTD2);
		if ( (lives == 0 && tick >= FLASH_ON) || (left && digit == 0) ) {
			PORTC = 0;
			} else {
			PORTC = seven_seg[digit] | (dp << 7);
		}
	}
	
	if (lives == 0) {
		PORTA = (PORTA & 0x0f) | (tick >= FLASH_ON ? 0 : 0xf0);
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
	move_cursor(score_x, score_y+1);
	set_display_attribute(TERM_RESET);
	printf_P(PSTR("Lives:      "));
	
	/* LEDS
	2 3 4 5 
	G O R G
	*/
	uint8_t leds = 0;
	/*set_display_attribute(TERM_BRIGHT);*/
	switch (lives) {
		case 4:
		leds = 0b1111;
		fast_set_display_attribute(FG_CYAN);
		draw_horizontal_line(score_y+1, score_x+8, score_x+11);
		break;
		
		case 3:
		leds = 0b0111;
		fast_set_display_attribute(FG_GREEN);
		draw_horizontal_line(score_y+1, score_x+9, score_x+11);
		break;
		
		case 2:
		leds = 0b0110;
		fast_set_display_attribute(FG_YELLOW);
		draw_horizontal_line(score_y+1, score_x+10, score_x+11);
		break;
		
		case 1:
		fast_set_display_attribute(FG_RED);
		draw_horizontal_line(score_y+1, score_x+11, score_x+11);
		leds = 0b0010;
		break;
		
		default:
		leds = 0;
		break;	
	}
	move_cursor(0, 0);
	PORTA = (PORTA & 0x0f) | (leds << 4);	
}

int32_t get_lives() {
/*	return 1;*/
	return lives;
}
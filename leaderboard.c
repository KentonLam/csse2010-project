/*
 * leaderboard.c
 *
 * Created: 21/05/2019 11:08:08 AM
 *  Author: Kenton
 */ 

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include "terminalio.h"
#include "serialio.h"

#define EEPROM_SIG 0xfade
#define SIG_ADDRESS (uint16_t *)20
#define SCORES_START (uint16_t *)40
#define NAMES_START (void *)60
#define MAX_LEADERBOARD 5
#define NAME_LEN 12

#define ESCAPE_CHAR 27
#define MISSING 0xefff

typedef struct afdjskl {
	uint16_t score;
	char name[NAME_LEN+1];	
} HighScore;

HighScore highscores[MAX_LEADERBOARD];

uint8_t numScores = 0;

void write_leaderboard(void) {
	for (uint8_t i = 0; i < MAX_LEADERBOARD; i++) {
		if (i >= numScores) {
			highscores[i].score = MISSING;
			strcpy(highscores[i].name, "--INVALIDx--");
		}
		eeprom_write_word(SCORES_START+2*i, highscores[i].score);
		eeprom_write_block(highscores[i].name, (void*)(NAMES_START+NAME_LEN*i), 12);
	}
}

static void reset_eeprom(void) {
	numScores = 0;
	write_leaderboard();
	eeprom_write_word(SIG_ADDRESS, EEPROM_SIG);
}

void print_leaderboard(uint8_t x, uint8_t y) {
	move_cursor(x, y);
	printf_P(PSTR("LEADERBOARD"));
	for (uint8_t i = 0; i < numScores; i++) {
		move_cursor(x, y+i+1);
		printf("%d. %-12s %4u", i+1, 
			highscores[numScores-i-1].name, highscores[numScores-i-1].score);
	}
	for (uint8_t n = 0; n < MAX_LEADERBOARD-numScores; n++) {
		move_cursor(x, y+n+1+numScores);
		printf_P(PSTR("%d. (empty)"), n+numScores+1);
	}
}

void init_leaderboard(void) {
	uint16_t signature = eeprom_read_word(SIG_ADDRESS);
	if (signature != EEPROM_SIG) {
		reset_eeprom();
	}
	numScores = 0;
	for (uint8_t i = 0; i < MAX_LEADERBOARD; i++) {
		highscores[i].score = eeprom_read_word(SCORES_START+2*i);
		if (highscores[i].score != MISSING) {
			numScores++;
		}
		eeprom_read_block(highscores[i].name, (void*)(NAMES_START+NAME_LEN*i), NAME_LEN);
	}
}

uint8_t made_leaderboard(uint16_t new_score) {
	for (uint8_t i = 0; i < MAX_LEADERBOARD; i++) {
		if (highscores[i].score == MISSING 
				|| new_score > highscores[i].score) {
			return 1;
		}
	}
	return 0;
}

void sort_leaderboard() {
	uint8_t i, j;
	HighScore temp;
	// from https://www.geeksforgeeks.org/insertion-sort/
	for (i = 1; i < numScores; i++) {
		/* invariant:  array[0..i-1] is sorted */
		j = i;
		/* customization bug: SWAP is not used here */
		temp = highscores[j];
		while (j > 0 && (highscores[j-1].score > temp.score)) {
			highscores[j] = highscores[j-1];
			j--;
		}
		highscores[j] = temp;
	}
}

void ask_name(uint16_t score) {
	char name[NAME_LEN+1] = {0};	
	uint8_t c_num = 0;
	
	char serial_input = -1;
	char escape_sequence_char = -1;
	uint8_t characters_into_escape_sequence = 0;
	
	show_cursor();
	clear_serial_input_buffer();
	printf_P(PSTR("____________\b\b\b\b\b\b\b\b\b\b\b\b"));
#ifndef _LOOP_FOR_NAME
	while (1) {
		escape_sequence_char = -1;
		if(serial_input_available()) {
			// Serial data was available - read the data from standard input
			serial_input = fgetc(stdin);
			/*printf("%d,", serial_input);*/
			// Check if the character is part of an escape sequence
			if(characters_into_escape_sequence == 0 && serial_input == ESCAPE_CHAR) {
				// We've hit the first character in an escape sequence (escape)
				characters_into_escape_sequence++;
				serial_input = -1; // Don't further process this character
			} else if(characters_into_escape_sequence == 1 && serial_input == '[') {
				// We've hit the second character in an escape sequence
				characters_into_escape_sequence++;
				serial_input = -1; // Don't further process this character
			} else if(characters_into_escape_sequence == 2) {
				// Third (and last) character in the escape sequence
				escape_sequence_char = serial_input;
				serial_input = -1;  // Don't further process this character - we
				// deal with it as part of the escape sequence
				characters_into_escape_sequence = 0;
			} else {
				// Character was not part of an escape sequence (or we received
				// an invalid second character in the sequence). We'll process
				// the data in the serial_input variable.
				characters_into_escape_sequence = 0;
			}
			
			if (serial_input == 0x7f || serial_input == 0x8) { // backspace
				if (c_num > 0) {
					
					c_num--;
					name[c_num] = '\0';
					printf_P(PSTR("\b_\b"));
				}
				continue;
			}
			if (serial_input == '\n') { // enter was pressed
				break;
			}
			
			if (c_num >= NAME_LEN) {
				continue;
			}
			
			if (('a' <= serial_input && serial_input <= 'z')
				|| ('A' <= serial_input && serial_input <= 'Z')
				|| (serial_input == ' ')) {
				printf_P(PSTR("%c"), serial_input);
				name[c_num++] = serial_input;
				continue;
			}
			
			if (0 && escape_sequence_char) {}
			
		}
	}
#endif

	uint8_t pos = numScores < MAX_LEADERBOARD ? numScores : 0;

	highscores[pos].score = score;
	strcpy(highscores[pos].name, name);
	if (numScores < MAX_LEADERBOARD) {
		numScores++;
	}
	hide_cursor();
	sort_leaderboard();
	write_leaderboard();
}
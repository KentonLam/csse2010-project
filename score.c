/*
 * score.c
 *
 * Written by Peter Sutton
 */

#include "score.h"
#include "serialio.h"

uint32_t score;

void init_score(void) {
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
	printf("Score: %3d", score);
}

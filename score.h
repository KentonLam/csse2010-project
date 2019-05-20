/*
 * score.h
 * 
 * Author: Peter Sutton
 */

#ifndef SCORE_H_
#define SCORE_H_

#include <stdint.h>

void init_score(void);
void add_to_score(int16_t value);
int32_t get_score(void);

void update_score_tick(void);

void change_lives(int8_t change);
int32_t get_lives(void);

#endif /* SCORE_H_ */
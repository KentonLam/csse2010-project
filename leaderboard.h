/*
 * leaderboard.h
 *
 * Created: 21/05/2019 11:08:17 AM
 *  Author: Kenton
 */ 


#ifndef LEADERBOARD_H_
#define LEADERBOARD_H_

void init_leaderboard(void);
uint8_t made_leaderboard(uint16_t new_score);
void print_leaderboard(uint8_t x, uint8_t y);

void ask_name(uint16_t score);



#endif /* LEADERBOARD_H_ */
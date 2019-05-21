/*
 * display.h
 *
 * Created: 21/05/2019 6:18:25 PM
 *  Author: Kenton
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#define CODE_BLACK (1)
#define CODE_RED (1<<3)
#define CODE_YELLOW (1<<2)
#define CODE_GREEN (1<<1)

void reset_frame();
void new_frame();
void set_pixel(uint8_t x, uint8_t y, uint8_t colour);
void draw_frame();

#endif /* DISPLAY_H_ */

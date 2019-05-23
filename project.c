/*
 * project.c
 *
 * Main file
 *
 * Author: Peter Sutton. Modified by Kenton Lam
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "ledmatrix.h"
#include "scrolling_char_display.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "score.h"
#include "timer0.h"
#include "joystick.h"
#include "game.h"

#include "leaderboard.h"

#define F_CPU 8000000L
#include <util/delay.h>

#include <assert.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void splash_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

// ASCII code for Escape character
#define ESCAPE_CHAR 27

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete
	splash_screen();
	
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	set_display_attribute(TERM_RESET);
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	
	init_leaderboard();
	init_joystick();
	init_sound();
	
	// Turn on global interrupts
	sei();
}

void splash_screen(void) {
	
	// Clear terminal screen and output a message
	clear_terminal();
	move_cursor(10,10);
	printf_P(PSTR("Asteroids"));
	move_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 project by Kenton Lam (45294583)"));
	
	// Output the scrolling message to the LED matrix
	// and wait for a push button to be pushed.
	ledmatrix_clear();
	while(1) {
		set_scrolling_display_text("45294583", COLOUR_ORANGE);
		// Scroll the message until it has scrolled off the 
		// display or a button is pushed
		while(scroll_display()) {
			_delay_ms(100);
			if(button_pushed() != NO_BUTTON_PUSHED || serial_input_available()) {
				return;
			}
		}
	}
}

void clear_all_input_buffers() {
	clear_serial_input_buffer(); // empty serial buffer
	while (button_pushed() != NO_BUTTON_PUSHED) {} // empty button butter
}

void new_game(void) {
	// Initialise the game and display
	
	
	// Clear the serial terminal
	clear_terminal();
	hide_cursor();
	set_display_attribute(FG_WHITE);
	draw_rectangle(X_LEFT, Y_TOP, X_RIGHT-X_LEFT+1, Y_BOTTOM-Y_TOP+1);
	
	initialise_game();
	
	
	set_display_attribute(TERM_RESET);
	set_display_attribute(TERM_REVERSE);
	/*set_display_attribute(FG_CYAN);*/
	move_cursor(X_TITLE, Y_TITLE);
	printf_P(PSTR("ASTEROIDS"));
	set_display_attribute(TERM_RESET);
	
	
// 	move_cursor(X_LEFT+1, Y_TOP+1);
// 	printf("X");
// 	move_cursor(X_RIGHT-1, Y_BOTTOM-1);
// 	printf("X");
// 	
// 	draw_horizontal_line(Y_TOP, X_LEFT, X_RIGHT);
// 	draw_horizontal_line(Y_BOTTOM, X_LEFT, X_RIGHT);
// 	
// 	draw_vertical_line(X_LEFT, Y_TOP, Y_BOTTOM);
// 	draw_vertical_line(X_RIGHT, Y_TOP, Y_BOTTOM);
	
	// Initialise the score
	init_score(X_SCORE, Y_SCORE);
	
	print_leaderboard(X_LEADERBOARD, Y_TOP);
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

uint8_t prev_joystick = 100;
uint32_t last_joystick_time = 0;
uint16_t joystick_interval = 500;

uint8_t check_joystick_move(uint8_t joystick) {
	uint32_t cur_time = get_current_time();
	
	if (joystick != prev_joystick) { // react instantly at first but set long delay.
		prev_joystick = joystick;
		last_joystick_time = cur_time;
		joystick_interval = 300;
		return joystick;
	}
	
	if (cur_time > last_joystick_time + joystick_interval) {
		joystick_interval = 100;
		last_joystick_time = cur_time;
		return joystick;
	}
	return 0;
}

void play_game(void) {
	uint32_t current_time, last_proj_move, last_asteroid_move;
	int8_t button, joy;
	char serial_input, escape_sequence_char;
	uint8_t characters_into_escape_sequence = 0;
	
	int16_t asteroidTick = 0;
	uint32_t pause_time = 0;
	
	// Get the current time and remember this as the last time the projectiles
    // were moved.
	current_time = get_current_time();
	last_proj_move = current_time;
	last_asteroid_move = current_time;
	
	get_joystick_input();
	
	// We play the game until it's over
	while(!is_game_over()) {
		
		// Check for input - which could be a button push or serial input.
		// Serial input may be part of an escape sequence, e.g. ESC [ D
		// is a left cursor key press. At most one of the following three
		// variables will be set to a value other than -1 if input is available.
		// (We don't initalise button to -1 since button_pushed() will return -1
		// if no button pushes are waiting to be returned.)
		// Button pushes take priority over serial input. If there are both then
		// we'll retrieve the serial input the next time through this loop
		serial_input = -1;
		escape_sequence_char = -1;
		button = button_pushed();
		joy = check_joystick_move(get_joystick_input());

		
		if(button == NO_BUTTON_PUSHED) {
			// No push button was pushed, see if there is any serial input
			if(serial_input_available()) {
				// Serial data was available - read the data from standard input
				serial_input = fgetc(stdin);
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
			}
		}
		
		// Process the input. 
		// Check for pause/unpause first.
		if (serial_input == 'p' || serial_input == 'P') {
			// Unimplemented feature - pause/unpause the game until 'p' or 'P' is
			// pressed again
			set_paused(is_paused() ^ 1);			
			move_cursor(2, 3);
			if (is_paused()) {
				pause_time = get_current_time();
				move_cursor(X_TITLE, Y_TITLE+1);
				fast_set_display_attribute(BG_YELLOW);
				fast_set_display_attribute(FG_BLACK);
				printf_P(PSTR("(Paused)"));
				fast_set_display_attribute(TERM_RESET);
			} else {
				current_time = get_current_time();
				last_asteroid_move += current_time-pause_time;
				last_proj_move += current_time-pause_time;
				move_cursor(X_TITLE, Y_TITLE+1);
				clear_to_end_of_line();
				clear_all_input_buffers();
				get_joystick_input();
			}
		}
		if (is_paused()) {
			continue;
		}
		
		if ((joy)==4 || button==3 || escape_sequence_char=='D' || serial_input=='L' || serial_input=='l') {
			// Button 3 pressed OR left cursor key escape sequence completed OR
			// letter L (lowercase or uppercase) pressed - attempt to move left			
			move_base(MOVE_LEFT);
		} else if((joy)==1||button==2 || escape_sequence_char=='A' || serial_input==' ') {
			// Button 2 pressed or up cursor key escape sequence completed OR
			// space bar pressed - attempt to fire projectile
			fire_projectile();
		} else if(button==1 || escape_sequence_char=='B') {
			// Button 1 pressed OR down cursor key escape sequence completed
			// Ignore at present
		} else if((joy)==2||button==0 || escape_sequence_char=='C' || serial_input=='R' || serial_input=='r') {
			// Button 0 pressed OR right cursor key escape sequence completed OR
			// letter R (lowercase or uppercase) pressed - attempt to move right
			move_base(MOVE_RIGHT);
		} else {};
		// else - invalid input or we're part way through an escape sequence -
		// do nothing
		
		current_time = get_current_time();
		if(!is_paused() && current_time >= last_proj_move + 100) {
			// 500ms (0.5 second) has passed since the last time we moved
			// the projectiles - move them - and keep track of the time we 
			// moved them
			advance_projectiles();
			last_proj_move = current_time;
		}
		
		if (PIND & (1<<PIND3)) {
			asteroidTick = 120 + 3000/(get_score()+4);
		} else {
			asteroidTick = 150 + 30000/(get_score()+20);
			if (asteroidTick < 550)
				asteroidTick = 550;
		}		
// 		if (get_score() > 1000 || asteroidTick < 180) {
// 			asteroidTick = 180;
// 		}
		if(serial_input=='x' || (!is_paused() && current_time >= last_asteroid_move + asteroidTick) ) {
			/*uint32_t c = get_current_time();*/
			advance_asteroids();
			/*printf("Render: %d     ", get_current_time() - c);*/
			last_asteroid_move = current_time;
		}
		
	}
	// We get here if the game is over.
}

void handle_game_over() {
	for (uint8_t y = 0; y < H_GAME_OVER+2; y++) {
		move_cursor(X_GAME_OVER+1, Y_GAME_OVER+y);
		clear_to_end_of_line();
	}
	
	set_display_attribute(FG_RED);
	draw_rectangle(X_GAME_OVER, Y_GAME_OVER, W_GAME_OVER+2, H_GAME_OVER+2);
	
	move_cursor(X_GAME_OVER+1, Y_GAME_OVER+1);
	printf_P(PSTR("GAME OVER"));
	
	_delay_ms(300);
	
	move_cursor(X_GAME_OVER+1, Y_GAME_OVER+2);
	printf_P(PSTR("Score: %d. "), get_score());
	if (made_leaderboard(get_score())) {
		set_display_attribute(TERM_BLINK);
		printf_P(PSTR("New highscore!"));
		set_display_attribute(TERM_RESET);
		move_cursor(X_GAME_OVER+1, Y_GAME_OVER+3);
		printf_P(PSTR("Name: "));

		ask_name(get_score());
		print_leaderboard(X_LEADERBOARD, Y_TOP);
	}
	_delay_ms(200);
	move_cursor(X_GAME_OVER+1,Y_GAME_OVER+4);
	printf_P(PSTR("Press any key to start over..."));
	clear_all_input_buffers();
	while(button_pushed() == NO_BUTTON_PUSHED && !serial_input_available()) {
		; // wait
	}
	
}

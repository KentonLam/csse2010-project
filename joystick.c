/*
 * joystick.c
 *
 * Created: 22/05/2019 6:21:52 PM
 *  Author: Kenton
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define DEAD_RADIUS 200

uint16_t value;
uint8_t adc_xy = 0;	/* 0 = x, 1 = y */

uint16_t x_centre;
uint16_t y_centre;

int8_t last_move = 0;
	
void init_joystick() {
			// Set up ADC - AVCC reference, right adjust
	// Input selection doesn't matter yet - we'll swap this around in the while
	// loop below.
	ADMUX = (1<<REFS0);
	// Turn on the ADC (but don't start a conversion yet). Choose a clock
	// divider of 64. (The ADC clock must be somewhere
	// between 50kHz and 200kHz. We will divide our 8MHz clock by 64
	// to give us 125kHz.)
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	
	
	ADMUX &= ~1; // using ADC0, XY.
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) {}
	x_centre = ADC; // read the value
	
	ADMUX |= 1; // using ADC0, XY.
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)) {}
	y_centre = ADC; // read the value
	
	// enable interrupts
	ADCSRA |= 1<<ADIE;
	ADCSRA |= (1<<ADSC);
}

ISR(ADC_vect) {
	uint16_t value = ADC;
	if (adc_xy && (value < x_centre-DEAD_RADIUS || value > x_centre+DEAD_RADIUS)) {
		
		last_move = value > x_centre ? 2 : 4;
	} else if (value < y_centre-DEAD_RADIUS || value > y_centre+DEAD_RADIUS) {
			
		last_move = value > y_centre ? 1 : 3;
	}
	adc_xy ^= 1;
	if (adc_xy) {
		ADMUX &= ~1;
	} else {
		ADMUX |= 1;
	}
	ADCSRA |= (1<<ADSC);
}

uint8_t get_joystick_input() {
	uint8_t move = last_move;
	last_move = 0;
	return move;
}
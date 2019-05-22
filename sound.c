/*
 * sound.c
 *
 * Created: 22/05/2019 7:28:54 PM
 *  Author: Kenton
 */ 
#include "sound.h"

#include <avr/io.h>
#define F_CPU 8000000UL
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static const uint16_t outputCompares[89] PROGMEM = {2016, 1894, 1786, 1689, 1603, 1524, 1420, 1359, 1276, 1202, 1136, 1078, 1008, 962, 906, 856, 801, 762, 718, 672, 638, 601, 568, 534, 508, 477, 450, 425, 401, 379, 357, 338, 319, 300, 284, 268, 253, 239, 226, 213, 201, 189, 179, 169, 159, 151, 142, 134, 127, 120, 113, 106, 100, 95, 90, 84, 80, 75, 71, 67, 63, 60, 56, 53, 50, 47, 45, 42, 40, 38, 36, 34, 32, 30, 28, 27, 25, 24, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 13};

typedef struct Note {
	uint8_t note;
	uint8_t duration;
} Note;

uint8_t startupToneLength;
Note startupTone[15] = {
	{NOTE_C4, 10}
};

void init_sound() {
	
	DDRD |= (1<<DDRD4);
	
	//
	TCCR1A = (1 << COM1B1) | (0 <<COM1B0) | (0 <<WGM11) | (1 << WGM10);
	TCCR1B =  (1<<CS11) | (1<<CS10) ;
	TCCR1B |= (1 << WGM13) | (0 << WGM12) ;
	
	OCR1A = outputCompares[NOTE_A4];
	OCR1B = outputCompares[NOTE_A4]/2;
}
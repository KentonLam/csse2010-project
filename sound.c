/*
 * sound.c
 *
 * Created: 22/05/2019 7:28:54 PM
 *  Author: Kenton
 */ 
#include "sound.h"
#include "timer0.h"

#include <avr/io.h>
#include <stdio.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

static const uint16_t outputCompares[] PROGMEM = {2016, 1894, 1786, 1689, 1603, 1524, 1420, 1359, 1276, 1202, 1136, 1078, 1008, 962, 906, 856, 801, 762, 718, 672, 638, 601, 568, 534, 508, 477, 450, 425, 401, 379, 357, 338, 319, 300, 284, 268, 253, 239, 226, 213, 201, 189, 179, 169, 159, 151, 142, 134, 127, 120, 113, 106, 100, 95, 90, 84, 80, 75, 71, 67, 63, 60, 56, 53, 50, 47, 45, 42, 40, 38, 36, 34, 32, 30, 28, 27, 25, 24, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 13};

#define TRACKS 6
#define NOTE_WORD(x, y) ((x<<8) | y)

volatile uint16_t musicIndexes[TRACKS] = { 0 };
uint16_t musicOffsets[TRACKS+1] = { 0 }; // initialised properly later
uint16_t musicLengths[TRACKS] = {6, 2, 2, 687, 4, 1};
static const uint16_t musics[6+2+2+687+4+1] PROGMEM = {
	// WINDOWS BOOT UP
	NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_DS4, 5), NOTE_WORD(NOTE_AS4, 16), NOTE_WORD(NOTE_GS4, 21), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_AS4, 32),
	// PROJECTILE HIT SOUND
	NOTE_WORD(NOTE_B4, 3), NOTE_WORD(NOTE_D4, 3),
	// BASE HIT SOUND
	NOTE_WORD(NOTE_G3, 8), NOTE_WORD(NOTE_C3, 8),
	// UN OWEN 
	NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 8), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_CS5, 3), NOTE_WORD(NOTE_AS4, 3), NOTE_WORD(NOTE_F4, 3), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 8), NOTE_WORD(NOTE_F4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_E4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_E4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_FS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_FS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_CS5, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_AS5, 46), NOTE_WORD(NOTE_SILENCE, 2), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_CS5, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_AS5, 46), NOTE_WORD(NOTE_SILENCE, 2), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_AS5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_DS5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_D5, 17), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_A4, 3), NOTE_WORD(NOTE_C5, 3), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_F4, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_F4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_G4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_G4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_CS5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_A5, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_D5, 17), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_A4, 3), NOTE_WORD(NOTE_C5, 3), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_F4, 4), NOTE_WORD(NOTE_D5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_F4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_G4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_C5, 4), NOTE_WORD(NOTE_G4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_CS5, 4), NOTE_WORD(NOTE_A4, 4), NOTE_WORD(NOTE_E5, 4), NOTE_WORD(NOTE_F5, 4), NOTE_WORD(NOTE_G5, 4), NOTE_WORD(NOTE_A5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_A5, 8), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_A5, 8), NOTE_WORD(NOTE_CS4, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C4, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_E4, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS4, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_FS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_FS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_CS4, 6), NOTE_WORD(NOTE_CS5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_C4, 6), NOTE_WORD(NOTE_C5, 6), NOTE_WORD(NOTE_G4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_E4, 6), NOTE_WORD(NOTE_E5, 6), NOTE_WORD(NOTE_DS4, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_SILENCE, 24), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_CS5, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_AS5, 46), NOTE_WORD(NOTE_SILENCE, 2), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_CS5, 23), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS4, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_C5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_F5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_CS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_DS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_G5, 11), NOTE_WORD(NOTE_SILENCE, 1), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_F5, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 6), NOTE_WORD(NOTE_C6, 6), NOTE_WORD(NOTE_CS6, 3), NOTE_WORD(NOTE_C6, 3), NOTE_WORD(NOTE_AS5, 6), NOTE_WORD(NOTE_GS5, 6), NOTE_WORD(NOTE_AS5, 46), 
	// WINDOWS SHUTDOWN
	NOTE_WORD(NOTE_GS5, 11), NOTE_WORD(NOTE_DS5, 11), NOTE_WORD(NOTE_GS4, 11), NOTE_WORD(NOTE_AS4, 16),
	// SILENCE
	NOTE_WORD(NOTE_SILENCE, 254)
	
};

volatile uint8_t curTrack = 0;

volatile uint8_t semiquavers = 0;
volatile uint8_t playing = 0;
volatile uint8_t playingBGM = 0;

void start_bgm() {
	playingBGM = 1;
	play_track(TRACK_TOUHOU);
}

void stop_bgm() {
	playingBGM = 0;
	playing = 0;	
}

void toggle_bgm() {
	if (!playingBGM) {
		playingBGM = 1;
		resume_track(TRACK_TOUHOU);
		try_unpause_music();
	} else {
		playingBGM = 0;
		if (curTrack == TRACK_TOUHOU) {
			play_track(TRACK_SILENCE);
		}
	}
}

void try_unpause_music() {
	if (PIND & (1<<PIND5)) {
		unpause_music();
	}
}

void unpause_music() {
	playing = 1;
	TCCR1B |= (1<<CS11) | (1<<CS10);
	DDRD |= (1<<DDRD4); // buzzer pin
	/*printf("UNPAUSE");*/
}

void pause_music() {
	playing = 0;
	TCCR1B &=  ~( (1<<CS11) | (1<<CS10) );
	DDRD &= ~(1<<DDRD4); // buzzer pin
}

void play_note(uint16_t note) {
	if (note != NOTE_SILENCE) {
 		OCR1A = pgm_read_word(outputCompares + note);
 		OCR1B = pgm_read_word(outputCompares + note)/2;
	} else {
		OCR1A = 100; // setting OCR1B > OCR1A will prevent buzzer from playing sound.
		OCR1B = 200;
	}
}

uint8_t extract_note(uint8_t track, uint16_t note_num) {
	return pgm_read_word(musics + musicOffsets[track]+ note_num) >> 8;
}

uint8_t extract_duration(uint8_t track, uint16_t note_num) {
	/*printf_P(PSTR("%x "), pgm_read_word(musics + TRACK_LEN*track + note_num));*/
	return pgm_read_word(musics + musicOffsets[track] + note_num) & 0xff;
}

void play_track(uint8_t track) {
	
	try_unpause_music();
	curTrack = track;
	/*_delay_ms(1000);*/
	musicIndexes[track] = 0;
	semiquavers=0;
	play_note(extract_note(track, 0));
// 	printf_P(PSTR("PLAYING%d\n"), musics[index][0].note);
}

void resume_track(uint8_t track) {
	curTrack = track;
	playing = 1;
}

void init_sound() {
	for (uint8_t i = 0; i < TRACKS; i++) {
		musicOffsets[i+1] = musicOffsets[i] + musicLengths[i];
	}
	
	PCICR |= 1<<PCIE3;
	PCMSK3 |= 1<<PCINT29;
	
	DDRD &= ~(1<<DDRD5); // mute pin
	DDRD |= (1<<DDRD4); // buzzer pin
	play_track(TRACK_SILENCE);
	//
	TCCR1A = (1 << COM1B1) | (0 <<COM1B0) | (1 <<WGM11) | (1 << WGM10);
	TCCR1B =  (1<<CS11) | (1<<CS10) ;
	TCCR1B |= (1 << WGM13) | (1 << WGM12) ;
// 	printf_P(PSTR("%x"), get_current_time());
// 	printf_P(PSTR("12titiating music\n"));
// 	printf_P(PSTR("412789 music\n"));
// 	printf_P(PSTR("2 music\n"));
// 	play_note(NOTE_C3);
// 	return;
	
	if (PIND & (1<<PIND5)) {
		unpause_music();
	} else {
		pause_music();
	}
}

void tick_sound() {
	/*printf("%d\n", semiquavers);*/
	if (!playing) { return; }
	
	semiquavers++;
	if (semiquavers >= extract_duration(curTrack, musicIndexes[curTrack])) {
		semiquavers = 0;
		musicIndexes[curTrack] = musicIndexes[curTrack]+1;
		/*printf("track %d, note %d\n", curTrack, musicIndexes[curTrack]);*/
		if (musicIndexes[curTrack] >= musicOffsets[curTrack+1]-musicOffsets[curTrack]) {
			// reached end of music.
			/*printf("ENDED");*/
			if (playingBGM) {
				if (curTrack == TRACK_TOUHOU) {
					play_track(TRACK_TOUHOU);
				} else {
					resume_track(TRACK_TOUHOU);
				}
			} else {
				/*curTrack = TRACK_TOUHOU*/;
				play_track(TRACK_SILENCE);
			}
		}
		
		play_note(extract_note(curTrack, musicIndexes[curTrack]));
	}
}

ISR(PCINT3_vect) {
	if (PIND & (1<<PIND5)) {
		unpause_music();
	} else {
		pause_music();
	}
}
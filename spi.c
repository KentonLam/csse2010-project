/*
 * spi.c
 *
 * Author: Peter Sutton
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "spi.h"

#define SPI_OUTPUT_BUFFER_SIZE 255
volatile uint8_t spi_out_buffer[SPI_OUTPUT_BUFFER_SIZE];
volatile uint8_t spi_out_insert_pos;
volatile uint8_t spi_bytes_in_out_buffer;
volatile uint8_t writing;


void spi_setup_master(uint8_t clockdivider) {
	spi_out_insert_pos = 0;
	spi_bytes_in_out_buffer = 0;
	writing = 0;
	
	// Set up SPI communication as a master
	// Make the SS, MOSI and SCK pins outputs. These are pins
	// 4, 5 and 7 of port B on the ATmega324A
	
	DDRB |= (1<<4)|(1<<5)|(1<<7);
	
	// Set the slave select (SS) line high
	PORTB |= (1<<4);
	
	// Set up the SPI control registers SPCR and SPSR:
	// - SPE bit = 1 (SPI is enabled)
	// - MSTR bit = 1 (Master Mode)
	SPCR0 = (1 << SPIE0)|(1<<SPE0)|(1<<MSTR0);
	
	// Set SPR0 and SPR1 bits in SPCR and SPI2X bit in SPSR
	// based on the given clock divider
	// Invalid values default to the slowest speed
	// We consider each bit in turn
	switch(clockdivider) {
		case 2:
		case 8:
		case 32:
			SPSR0 = (1<<SPI2X0);
			break;
		default:
			SPSR0 = 0;
			break;
	}
	switch(clockdivider) {
		case 128:
			SPCR0 |= (1<<SPR00);
			// Note this flows through to the next code block
		case 32:
		case 64:
			SPCR0 |= (1<<SPR10);
			break;
		case 8:
		case 16:
			SPCR0 |= (1<<SPR00);
			break;
	}
	
	// Take SS (slave select) line low
	PORTB &= ~(1<<4);
	sei();
}

uint8_t spi_send_byte(uint8_t byte) {
	if (!writing) {
		SPDR0 = byte;
		writing = 1;
		return 1;
	}
	
	uint8_t interrupts_enabled;
	
	/* If the buffer is full and interrupts are disabled then we
	 * abort - we don't output the character since the buffer will
	 * never be emptied if interrupts are disabled. If the buffer is full
	 * and interrupts are enabled then we loop until the buffer has 
	 * enough space. The bytes_in_buffer variable will get modified by the
	 * ISR which extracts bytes from the buffer.
	*/
	interrupts_enabled = bit_is_set(SREG, SREG_I);
	while(spi_bytes_in_out_buffer >= SPI_OUTPUT_BUFFER_SIZE) {
		if(!interrupts_enabled) {
			return 1;
		}		
		/* else do nothing */
	}
	
	/* Add the character to the buffer for transmission if there
	 * is space to do so. We advance the insert_pos to the next
	 * character position. If this is beyond the end of the buffer
	 * we wrap around back to the beginning of the buffer 
	 * NOTE: we disable interrupts before modifying the buffer. This
	 * prevents the ISR from modifying the buffer at the same time.
	 * We reenable them if they were enabled when we entered the
	 * function.
	*/	
	cli();
	spi_out_buffer[spi_out_insert_pos++] = byte;
	spi_bytes_in_out_buffer++;
	if(spi_out_insert_pos == SPI_OUTPUT_BUFFER_SIZE) {
		/* Wrap around buffer pointer if necessary */
		spi_out_insert_pos = 0;
	}
	/* Reenable interrupts (UDR Empty interrupt may have been
	 * disabled) - we ensure it is now enabled so that it will
	 * fire and deal with the next character in the buffer. */
	/*SPCR0 |= (1 << SPIE0);*/
	if(interrupts_enabled) {
		sei();
	}
	return 0;
}

ISR(SPI_STC_vect) {
	if (spi_bytes_in_out_buffer > 0) {
		// Write out the byte to the SPDR0 register. This will initiate
		// the transfer. We then wait until the most significant byte of
		// SPSR0 (SPIF0 bit) is set - this indicates that the transfer is
		// complete. (The final read of SPSR0 followed by a read of SPDR0
		// will cause the SPIF bit to be reset to 0. See page 173 of the
		// ATmega324A datasheet.)
		char c;
		if(spi_out_insert_pos - spi_bytes_in_out_buffer < 0) {
			/* Need to wrap around */
			c = spi_out_buffer[spi_out_insert_pos - spi_bytes_in_out_buffer
			+ SPI_OUTPUT_BUFFER_SIZE];
			} else {
			c = spi_out_buffer[spi_out_insert_pos - spi_bytes_in_out_buffer];
		}
		spi_bytes_in_out_buffer--;
		SPDR0 = c;
		writing = 1;
	} else {
		writing = 0;
		/*SPCR0 &= ~(1<<SPIE0);*/
	}
}
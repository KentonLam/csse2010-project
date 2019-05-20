/*
 * sort.c
 *
 * Created: 20/05/2019 10:54:25 PM
 *  Author: Kenton
 */ 
#include "stdio.h"

#define GET_Y_POSITION(posn)	((posn) & 0x0F)

/*
Implementation of insertion sort keyed on the y values. 
*/
void asteroid_sort(uint8_t *array, uint8_t numAsteroids) {
	uint8_t i, j;
	uint8_t temp, key;
	
	for (i = 1; i < numAsteroids; i++) {
		/* invariant:  array[0..i-1] is sorted */
		j = i;
		/* customization bug: SWAP is not used here */
		temp = array[j];
		key = GET_Y_POSITION(temp);
		while (j > 0 && (GET_Y_POSITION(array[j-1]) > key)) {
			array[j] = array[j-1];
			j--;
		}
		array[j] = temp;
	}
}

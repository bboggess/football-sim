#include <stdlib.h>
#include "utils.h"

unsigned int rollDice(unsigned int numDice, bool breakaway) {
	int total = 0;
	while (numDice-- > 0) {
		int roll = rand() % NUM_SIDES + 1;
		if (breakaway && roll == 6)
			numDice++;
		total += roll;
	}

	return total;
}

unsigned int rollDice(unsigned int numDice) {
	return rollDice(numDice, false);
} 

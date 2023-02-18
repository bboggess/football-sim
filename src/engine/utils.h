#ifndef __UTILS_H
#define __UTILS_H

/* number of sides on our dice. Might want to change at some point */
#define NUM_SIDES 6

/**
 * Returns the sum of some number of six sided dic being rolled. When set, the
 * optional breakaway flag causes all 6's to result in a bonus roll.
 */
unsigned int rollDice(unsigned int numDice, bool breakaway);
unsigned int rollDice(unsigned int numDice);

#define MIN(A,B) (((A) > (B)) ? (B) : (A))

#endif
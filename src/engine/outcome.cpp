#include "outcome.h"
#include "utils.h"

/**
 * Used to add a fumble to the end of a play. Call this after a PlayOutcome
 * has been fully constructed by some other means.
 */
static void addFumble(PlayOutcome* outcome)
{
    int result = rollDice(1, false) - rollDice(1, false);
    if (result > 0) {
        outcome->changePoss = true;
        outcome->result = FUMBLE;
        if (result == 3)
            outcome->yardsGained -= rollDice(2, true);
        else if (result == 4)
            outcome->yardsGained -= rollDice(2, true) + 10;
        else if (result == 5)
            outcome->yardsGained -= rollDice(2, true) + 20;
    }
}

/**
 * Returns play ending in an interception.
 */
static inline PlayOutcome* interception()
{
    return new PlayOutcome(INTERCEPTION,
        rollDice(3, true) - rollDice(2, true),
        true,
        false);
}

/**
 * Returns play ending in incomplete pass.
 */
static inline PlayOutcome* incomplete()
{
    return new PlayOutcome(INCOMPLETE_PASS, 0, false, false);
}

/**
 * Returns a play resulting in a completed pass.
 */
static inline PlayOutcome* completion(unsigned int baseGain,
    unsigned int additionalDice,
    bool breakaway)
{
    return new PlayOutcome(COMPLETED_PASS,
        baseGain + rollDice(additionalDice, breakaway),
        false,
        false);
}

/**
 * Returns a play ending in a sack, with no fumble.
 */
static inline PlayOutcome* sack()
{
    return new PlayOutcome(SACK, -2 - rollDice(1, true), false, false);
}

static inline PlayOutcome* qbScramble()
{
    return new PlayOutcome(HANDOFF, rollDice(2, true) - 4, false, false);
}

PlayOutcome* OutcomeGenerator::genOutcome()
{
    PlayOutcome* toReturn = new PlayOutcome(result,
        baseYards + rollDice(numDice, breakaway),
        turnover,
        autoTd);

    if (isFumble)
        addFumble(toReturn);

    return toReturn;
}

PlayOutcome* MishapGenerator::genOutcome()
{
    PlayOutcome* outcome;
    unsigned int roll = rollDice(2, false);

    switch (roll) {
    case 2:
    case 3:
    case 4:
    case 5:
        outcome = sack();
        addFumble(outcome);
        break;
    case 6:
        outcome = interception();
    case 7:
    case 8:
        outcome = incomplete();
        break;
    case 9:
    case 10:
    case 11:
        outcome = completion(0, 2, true);
        addFumble(outcome);
        break;
    case 12:
        outcome = completion(0, 3, true);
        break;
    }

    return outcome;
}

/**
 * QB gets pressured when dropping back. Returns a random event to follow this.
 */
PlayOutcome* PressureGenerator::genOutcome()
{
    PlayOutcome* outcome;
    unsigned int roll = rollDice(2, false);

    switch (roll) {
    case 2:
    case 3:
        outcome = interception();
        break;
    case 4:
    case 5:
    case 6:
        outcome = sack();
        break;
    case 7:
    case 8:
        outcome = incomplete();
        break;
    case 9:
        outcome = qbScramble();
    case 10:
        outcome = completion(0, 1, true);
        break;
    case 11:
        outcome = completion(0, 2, true);
        break;
    case 12:
        outcome = completion(0, 3, true);
        break;
    }

    return outcome;
}

#include "game.h"
#include "playcall.h"
#include "utils.h"

#include <iostream>

Play::Play(PlayCall offense, PlayCall defense, Situation* sit)
{
    offCall = offense;
    defCall = defense;
    context = sit;
}

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
 * Creates a new PlayOutcome pointer with the given members.
 */
static PlayOutcome* newOutcome(PlayResult result, int yardsGained, bool changePoss,
    bool touchdown)
{
    PlayOutcome* outcome = new PlayOutcome();

    outcome->result = result;
    outcome->yardsGained = yardsGained;
    outcome->changePoss = changePoss;
    outcome->touchdown = touchdown;

    return outcome;
}

/**
 * Returns an automatic defensive touchdown, e.g. after an especially bad
 * offensive roll. The PlayResult argument allows us to differentiate between a
 * fumble and a pick 6.
 */
static inline PlayOutcome* defensiveTouchdown(PlayResult result)
{
    return newOutcome(result, 0, true, true);
}

/**
 * Returns an automatic offensive touchdown, which can occur e.g. after a
 * perfect dice roll. The PlayResult argument will differentiate between a
 * run, short pass, or deep bomb for the score.
 */
static inline PlayOutcome* offensiveTouchdown(PlayResult result)
{
    return newOutcome(result, 0, false, true);
}

/**
 * Returns play ending in an interception.
 */
static inline PlayOutcome* interception()
{
    return newOutcome(INTERCEPTION, rollDice(3, true) - rollDice(2, true), true,
        false);
}

/**
 * Returns play ending in incomplete pass.
 */
static inline PlayOutcome* incomplete()
{
    return newOutcome(INCOMPLETE_PASS, 0, false, false);
}

/**
 * Returns a play resulting in a completed pass.
 */
static inline PlayOutcome* completion(unsigned int baseGain, unsigned int additionalDice,
    bool breakaway)
{
    return newOutcome(COMPLETED_PASS, baseGain + rollDice(additionalDice, breakaway),
        false, false);
}

/**
 * Returns a play ending in a sack, with no fumble.
 */
static inline PlayOutcome* sack()
{
    return newOutcome(SACK, -2 - rollDice(1, true), false, false);
}

static inline PlayOutcome* qbScramble()
{
    return newOutcome(HANDOFF, rollDice(2, true) - 4, false, false);
}

/**
 * QB gets pressured when dropping back. Returns a random event to follow this.
 */
static PlayOutcome* qbPressure()
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

/**
 * Returns a play ending in a random mishap.
 */
static PlayOutcome* mishap()
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
 * Returns result of a handoff. The base is the base number of yards gained,
 * plus the sum of an additional number of dice. The negative flag indicates that
 * the play lost yards, and the multiplier divides the roll of the dice.
 */
static inline PlayOutcome* handoff(int base,
    unsigned int additionalDice,
    bool negative,
    int multiplier,
    bool breakaway)
{
    int roll = rollDice(additionalDice);
    roll += multiplier - 1;
    if (negative)
        roll = -roll;
    return newOutcome(HANDOFF, base + roll / multiplier, false, false);
}

/**
 * A streamlined version of handoff that looks more like completion().
 * Ignores the multiplier and negative yard flag.
 */
static inline PlayOutcome* handoff(int base, unsigned int additionalDice,
    bool breakaway)
{
    return handoff(base, additionalDice, false, 1, breakaway);
}

static inline PlayOutcome* fumbledSnap()
{
    PlayOutcome* outcome = newOutcome(SACK, -1, false, false);
    addFumble(outcome);
    return outcome;
}

/* Flags for disabling/enabling breakaway roll */
#define ALWAYS 1
#define NEVER -1
#define DEFAULT 0

/* An array containing all possible defensive modifiers.
 * The indexing goes [offensive call][defensive call][dice roll]
 */
static const int DEFENSIVE_MODS[][3][11] = {
    { { 1, 0, 0, 0, 0, 0, 0, -1, -1, -1, -2 },
        { 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, -1 },
        { 1, 1, 0, 0, 0, 0, 0, 0, 0, -1, -1 } },
    { { 1, 1, 0, 0, 0, 0, 0, 0, 0, -1, -1 },
        { 1, 0, 0, 0, 0, 0, 0, -1, -1, -1, -2 },
        { 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, -1 } },
    { { 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, -1 },
        { 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, -1 },
        { 1, 0, 0, 0, 0, 0, 0, -1, -1, -1, -2 } }
};

/**
 * Calculates modifier to add to offensive roll based on defensive play call.
 *
 * The breakaway modifier can be set to ALWAYS, NEVER, or DEFAULT.
 */
static int calcDefModifier(PlayCall offense, PlayCall defense, int& breakaway)
{
    // My play calling logic on fourth down is not fantastic...
    if (offense == PUNT || defense == PUNT || offense == FIELD_GOAL)
        return 0;

    unsigned int roll = rollDice(2, false);

    // We need to deal with setting breakaway for a select few special cases.
    // Really don't think this gets any better
    switch (offense) {
    case RUN:
        if (defense == RUN) {
            if (roll == 8 || roll == 11)
                breakaway = NEVER;
        } else if (defense == SHORT_PASS || defense == LONG_PASS) {
            if (roll == 3 || roll == 6)
                breakaway = ALWAYS;
        }
        break;

    case SHORT_PASS:
        if (defense == RUN) {
            if (roll == 4)
                breakaway = ALWAYS;
            else if (roll == 10)
                breakaway = NEVER;
        } else if (defense == SHORT_PASS) {
            if (roll == 8 || roll == 11)
                breakaway = NEVER;
        } else if (defense == LONG_PASS) {
            if (roll == 3 || roll == 6)
                breakaway = ALWAYS;
        }
        break;

    case LONG_PASS:
        if (defense == RUN) {
            if (roll == 3 || roll == 6)
                breakaway = ALWAYS;
        } else if (defense == SHORT_PASS) {
            if (roll == 4)
                breakaway = ALWAYS;
            else if (roll == 10)
                breakaway = NEVER;
        } else if (defense == LONG_PASS) {
            if (roll == 8 || roll == 11)
                breakaway = NEVER;
        }
    }

    return DEFENSIVE_MODS[offense][defense][roll - 2];
}

/**
 * Calculate outcome of a short pass play based on value of dice roll.
 */
static PlayOutcome* shortPassOutcome(unsigned int roll)
{
    PlayOutcome* outcome;
    switch (roll) {
    case 1:
    case 2:
        outcome = defensiveTouchdown(INTERCEPTION);
        break;
    case 3:
    case 4:
        outcome = interception();
        break;
    case 5:
        outcome = mishap();
        break;
    case 6:
    case 7:
    case 9:
        outcome = incomplete();
        break;
    case 8:
        outcome = qbPressure();
        break;
    case 10:
        outcome = completion(0, 1, false);
        break;
    case 11:
        outcome = completion(1, 1, false);
        break;
    case 12:
        outcome = completion(0, 2, false);
        break;
    case 13:
        outcome = completion(0, 2, true);
        break;
    case 14:
        outcome = completion(5, 2, true);
        break;
    case 15:
        outcome = completion(10, 2, true);
        break;
    case 16:
        outcome = completion(15, 2, true);
        break;
    case 17:
        outcome = completion(30, 3, true);
        break;
    case 18:
        outcome = completion(50, 3, true);
        break;
    case 19:
    case 20:
        outcome = offensiveTouchdown(COMPLETED_PASS);
        break;
    default:
        // TODO error here
        break;
    }

    return outcome;
}

/**
 * Calculate outcome of a long pass play based on value of dice roll.
 */
static PlayOutcome* longPassOutcome(unsigned int roll)
{
    PlayOutcome* outcome;
    switch (roll) {
    case 1:
    case 2:
        outcome = defensiveTouchdown(INTERCEPTION);
        break;
    case 3:
    case 4:
        outcome = interception();
        break;
    case 5:
        outcome = mishap();
        break;
    case 6:
        outcome = sack();
        break;
    case 7:
    case 9:
    case 10:
    case 11:
        outcome = incomplete();
        break;
    case 8:
        outcome = qbPressure();
        break;
    case 12:
        outcome = completion(2, 2, false);
        break;
    case 13:
        outcome = completion(0, 3, true);
        break;
    case 14:
        outcome = completion(5, 3, true);
        break;
    case 15:
        outcome = completion(10, 3, true);
        break;
    case 16:
        outcome = completion(15, 3, true);
        break;
    case 17:
        outcome = completion(40, 4, true);
        break;
    case 18:
    case 19:
    case 20:
        outcome = offensiveTouchdown(COMPLETED_PASS);
        break;
    default:
        // TODO error
        break;
    }

    return outcome;
}

/**
 * Calculate outcome of a running play based on value of dice roll.
 */
static PlayOutcome* runOutcome(unsigned int roll)
{
    PlayOutcome* outcome;
    switch (roll) {
    case 1:
    case 2:
        outcome = defensiveTouchdown(FUMBLE);
        break;
    case 3:
        outcome = fumbledSnap();
        break;
    case 4: {
        /* We need to roll again to determine where the fumble occurs. BUT
           if you roll another 3 or 4, this weird cyle of fumbling starts and
           I would like to avoid this entirely. */
        unsigned int spotOfFumble = rollDice(3, false);
        if (spotOfFumble < 5)
            spotOfFumble = 5;
        outcome = runOutcome(spotOfFumble);
        addFumble(outcome);
    } break;
    case 5:
        outcome = handoff(0, 1, true, 1, false);
        break;
    case 6:
        outcome = handoff(0, 1, true, 2, false);
        break;
    case 7:
        outcome = handoff(0, 1, true, 3, false);
        break;
    case 8:
        outcome = handoff(0, 0, false);
        break;
    case 9:
    case 10:
        outcome = handoff(0, 1, false, 2, false);
        break;
    case 11:
        outcome = handoff(0, 1, false);
        break;
    case 12:
        outcome = handoff(1, 1, false);
        break;
    case 13:
        outcome = handoff(0, 2, false);
        break;
    case 14:
        outcome = handoff(0, 2, true);
        break;
    case 15:
        outcome = handoff(0, 3, true);
        break;
    case 16:
        outcome = handoff(5, 3, true);
        break;
    case 17:
        outcome = handoff(20, 4, true);
        break;
    case 18:
        outcome = handoff(40, 4, true);
        break;
    case 19:
    case 20:
        outcome = offensiveTouchdown(HANDOFF);
        break;
    default:
        // TODO error
        break;
    }

    return outcome;
}

/**
 * Takes in the roll and returns the distance the punt travels in the air.
 */
static int getPuntDistance(unsigned int roll)
{
    int distance;

    switch (roll) {
    case 2:
        distance = rollDice(2, true) + 20;
        break;
    case 3:
        distance = rollDice(2, true) + 25;
        break;
    case 4:
        distance = rollDice(2, true) + 30;
        break;
    case 5:
    case 6:
    case 7:
    case 8:
        distance = rollDice(3, true) + 30;
        break;
    case 9:
    case 10:
        distance = rollDice(3, true) + 35;
        break;
    case 11:
        distance = rollDice(3, true) + 40;
        break;
    case 12:
        distance = rollDice(3, true) + 45;
        break;
    }

    return distance;
}

/**
 * Determines the number of yards gained on the punt return. This requires
 * knowing the value of the dice roll used for determining the kick's distance.
 */
static int getPuntReturn(unsigned int distanceRoll)
{
    unsigned int roll = rollDice(1, false);
    int returnYards;

    switch (roll) {
    case 1:
        if (distanceRoll == 2) {
            returnYards = 0;
        } else if (distanceRoll <= 6) {
            returnYards = rollDice(2, true);
        } else {
            returnYards = rollDice(3, true);
        }
        break;
    case 2:
        if (distanceRoll == 2) {
            returnYards = 0;
        } else if (distanceRoll <= 5) {
            returnYards = rollDice(2, true);
        } else {
            returnYards = rollDice(3, true);
        }
        break;
    case 3:
        if (distanceRoll <= 3) {
            returnYards = 0;
        } else if (distanceRoll <= 8) {
            returnYards = rollDice(2, true);
        } else {
            returnYards = rollDice(3, true);
        }
        break;
    case 4:
        if (distanceRoll <= 3) {
            returnYards = 0;
        } else if (distanceRoll <= 10) {
            returnYards = rollDice(2, true);
        } else {
            returnYards = rollDice(3, true);
        }
        break;
    case 5:
        if (distanceRoll <= 8) {
            returnYards = 0;
        } else if (distanceRoll <= 10) {
            returnYards = rollDice(2, true);
        } else if (distanceRoll <= 11) {
            returnYards = rollDice(3, true);
        } else {
            returnYards = 0;
        }
        break;
    case 6:
        returnYards = 0;
        break;
    }

    return returnYards;
}

/* Helper method for fieldGoalOutcome(). Decides what the minimum roll is for a
 * successful field goal attempt from the given yard line
 */
static inline unsigned int getMadeKickThresh(unsigned int fieldPos)
{
    unsigned int threshold;
    fieldPos = 100 - fieldPos;

    if (fieldPos <= 2)
        threshold = 5;
    else if (fieldPos <= 12)
        threshold = 6;
    else if (fieldPos <= 22)
        threshold = 7;
    else if (fieldPos <= 27)
        threshold = 9;
    else if (fieldPos <= 32)
        threshold = 10;
    else if (fieldPos <= 35)
        threshold = 11;
    else if (fieldPos <= 37)
        threshold = 12;
    else if (fieldPos <= 39)
        threshold = 13;
    else if (fieldPos <= 42)
        threshold = 14;
    else if (fieldPos <= 45)
        threshold = 15;
    else if (fieldPos <= 47)
        threshold = 16;
    else if (fieldPos <= 49)
        threshold = 17;
    else
        threshold = 18;

    return threshold;
}

/**
 * Calculates the result of a field goal attempt. Right now, what can happen is
 * either a FIELD_GOAL_MADE or a FIELD_GOAL_MISS. There is not yet an option for
 * a blocked attempt. If FIELD_GOAL_MISS, the changePoss flag is set.
 */
static PlayOutcome* fieldGoalOutcome(Situation* context)
{
    unsigned int roll = rollDice(3);
    unsigned int threshold = getMadeKickThresh(context->fieldPos);

    if (roll >= threshold)
        return newOutcome(FIELD_GOAL_MADE, 0, false, false);
    else
        return newOutcome(FIELD_GOAL_MISS, 0, true, false);
}

/**
 * Calculates the result of a punt. The PlayOutcome returned always has result
 * field PUNT_RETURN, the distance field tells where to move to set the ball
 * relative to the line of scrimmage at the time of the punt, and the changePoss
 * flag is set to true.
 */
static PlayOutcome* puntOutcome()
{
    unsigned int roll = rollDice(2, false);
    int distance = getPuntDistance(roll);

    int returnYards = getPuntReturn(roll);

    return newOutcome(PUNT_RETURN, distance - returnYards, true, false);
}

PlayOutcome* Play::runPlay()
{
    int breakaway = DEFAULT;
    int modifier = calcDefModifier(offCall, defCall, breakaway);
    int result = rollDice(3, false) + modifier;

    PlayOutcome* outcome;
    switch (offCall) {
    case SHORT_PASS:
        outcome = shortPassOutcome(result);
        break;
    case LONG_PASS:
        outcome = longPassOutcome(result);
        break;
    case RUN:
        outcome = runOutcome(result);
        break;
    case PUNT:
        outcome = puntOutcome();
        break;
    case FIELD_GOAL:
        outcome = fieldGoalOutcome(context);
        break;
    }

    // Handle the cases where the ball crosses either goal line
    if (outcome->yardsGained + context->fieldPos >= 100) {
        outcome->touchdown = true;
        outcome->yardsGained = 100 - context->fieldPos;
    } else if (outcome->yardsGained + context->fieldPos <= 0) {
        outcome->yardsGained = -context->fieldPos;
        if (outcome->changePoss)
            outcome->touchdown = true;
        else
            // Safety should be here
            ;
    }

    // For presentation purposes, it's nice not to score a 50 yard
    // touchdown from the goal line.
    if (outcome->touchdown) {
        outcome->yardsGained = outcome->changePoss ? context->fieldPos : 100 - context->fieldPos;
    }

    return outcome;
}
#include "team.h"
#include "../learn/model.h"
#include "utils.h"

// TODO: rewrite this to be simpler
static inline bool shouldPunt(Situation* sit)
{
    unsigned int roll = rollDice(1, false);
    return sit->down == FOURTH && sit->fieldPos <= 57 && (sit->distance > 2 || sit->fieldPos <= 40 || (sit->fieldPos <= 50 && sit->distance == 1 && roll == 6) || (sit->fieldPos <= 57 && ((sit->distance == 1 && roll > 1) || roll == 6)));
}

// TODO: more clever field goal kicking
static inline bool shouldKick(Situation* sit)
{
    return sit->down == FOURTH && sit->fieldPos >= 60;
}

/*
 * Calls a play using the machine learning model.
 * Look how much nicer than that fucking abomination using dice rolls.
 */
PlayCall AITeam::callPlay(Situation* situation)
{
    // Right now the model only takes into account offensive snaps,
    // and so it's easier to just have separate punt logic.
    if (shouldPunt(situation)) {
        return PUNT;
    } else if (shouldKick(situation)) {
        return FIELD_GOAL;
    } else {
        return getPlayCall(situation);
    }
}

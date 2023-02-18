#ifndef __OUTCOME_H
#define __OUTCOME_H

/**
 * A list of the possible results of each play, which can be shown to the
 * user, used for clock management, etc. Some of these should probably be able
 * to be chained together.
 */
enum PlayResult {
    COMPLETED_PASS,
    INCOMPLETE_PASS,
    FUMBLE,
    INTERCEPTION,
    HANDOFF,
    SACK,
    KICK_RETURN,
    FIELD_GOAL_MADE,
    FIELD_GOAL_MISS,
    PUNT_RETURN,
    FIELD_GOAL_BLOCK,
    PAT_MADE,
    PAT_MISS,
    TWO_PT_MADE,
    TWO_PT_MISS
};

/**
 * A struct containing all info on the outcome of a play which is needed by
 * any other part of the engine.
 *
 * The touchdown member signifies that a touchdown has been scored on the play.
 *
 * No matter what, yardsGained can be added to the previous play's starting
 * position for the new field position, even after a turnover.
 */
struct PlayOutcome {
    PlayResult result;
    int yardsGained;
    bool changePoss;
    bool touchdown;

    PlayOutcome(PlayResult res, int yards, bool turnover, bool td)
        : result(res)
        , yardsGained(yardsGained)
        , changePoss(turnover)
        , touchdown(td) {};
};

class OutcomeGenerator {
protected:
    int baseYards;
    unsigned int numDice;
    bool breakaway;
    unsigned int breakawayLimit;
    bool turnover;
    bool autoTD;

public:
    OutcomeGenerator(int base, unsigned int dice, bool breakaway)
        : baseYards(base)
        , numDice(dice)
        , breakaway(breakaway) {};
    OutcomeGenerator()
        : OutcomeGenerator(0, 0, false) {};
    virtual PlayOutcome* genOutcome();
};

class RunOutcomeGenerator : public OutcomeGenerator {

public:
    RunOutcomeGenerator(int base, unsigned int dice, bool breakaway)
        : OutcomeGenerator(base, dice, breakaway)
    {
        breakawayLimit = 4;
        result = HANDOFF;
    }

    RunOutcomeGenerator()
        : OutcomeGenerator()
    {
        result = HANDOFF;
        autoTD = true;
    }
};

class PassOutcomeGenerator : public OutcomeGenerator {

public:
    PassOutcomeGenerator(int base, unsigned int dice, bool breakaway)
        : OutcomeGenerator(base, dice, breakaway)
    {
        breakawayLimit = 4;
        result = COMPLETED_PASS;
    }

    PassOutcomeGenerator()
        : OutcomeGenerator()
    {
        result = COMPLETED_PASS;
        autoTD = true;
    }
};

class IncompletionGenerator : public PassOutcomeGenerator {
public:
    IncompletionGenerator()
        : OutcomeGenerator()
    {
        result = INCOMPLETE_PASS;
    };
};

class InterceptionOutcomeGenerator : public PassOutcomeGenerator {
public:
    InterceptionOutcomeGenerator(int base, unsigned int dice, bool breakaway)
        : OutcomeGenerator(base, dice, breakaway)
    {
        breakawayLimit = 2;
        changePoss = true;
        result = INTERCEPTION;
    }
};

class MishapGenerator : public OutcomeGenerator {
public:
    PlayOutcome* genOutcome();
};

class PressureGenerator : public OutcomeGenerator {
public:
    PlayOutcome* genOutcome();
};

#endif

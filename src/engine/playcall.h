#ifndef __PLAYCALL_H
#define __PLAYCALL_H

struct Situation;
/**
 * These are the possible playcall types, for both offense and defense.
 */
enum PlayCall { RUN, SHORT_PASS, LONG_PASS, PUNT, FIELD_GOAL };

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
};

/**
 * This is probably not needed. Might delete.
 *
 * Contains enough info about a play to be logged in the play-by-play and also
 * calculate the outcome of a given play.
 */
class Play {
private:
	PlayCall offCall;
	PlayCall defCall;
	Situation *context;
public:
	Play(PlayCall offense, PlayCall defense, Situation *sit);
	/**
	 * Calculates the outcome of a given play based on both teams' playcalls.
	 */
	PlayOutcome *runPlay();
};

#endif

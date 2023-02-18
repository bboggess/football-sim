#ifndef __GAME_STATES_H
#define __GAME_STATES_H

#include "states.h"
#include "game.h"

/**
 * This header contains all possible states a football game can exist in.
 * Each State has logic for when you first enter the state, when you exit
 * the state, and for executing one play (or equivalent) from that state.
 *
 * These should be understood as the state of a game before a play is run,
 * with the exception of Touchdown, Halftime, and Final.
 *
 * All state control flow is handled in these classes.
 *
 * IMPORTANT NOTE: All of these classes use the singleton pattern. Use the
 * static getInstance() method to access. No further remarks on this.
 */

/* Represents a game where the teams have lined up for a kickoff.
 * It sets up, executes the kickoff, and then hands the state machine
 * off to PlayFromScrimmage.
 */
class Kickoff : public State<Game> {
private:
	Kickoff() {}

public:
	static Kickoff *getInstance();
	/* Sets the ball on the 35 yard line for the kick */
	void enter(Game *game);
	/* Swaps possession to the other team, 1st and 10 at their own 25 */
	void execute(Game *game);
	void exit(Game *game);
};

/* Represents a PAT attempt, either a kick or a two point attempt. The offense
 * is able to call either a kick or go for two, and the score is updated
 * accordingly before moving the state to kickoff.
 */
class ExtraPoint : public State<Game> {
private:
	ExtraPoint() {}

public:
	static ExtraPoint *getInstance();
	/* Sets up at the 3 yard line for the extra point attempt */
	void enter(Game *game);
	/* Gets playcalls and simulates the PAT. Updates scores and moves control
	 * to the kickoff
	 */
	void execute(Game *game);
	void exit(Game *game);
};

/* Represents any typical play, where the offense snaps the ball and a touchdown
 * may be scored. The game will primarily exist in this state.
 *
 * These don't currently keep track of the down and distance, but I plan to move
 * towards that.
 */
class PlayFromScrimmage : public State<Game> {
private:
	PlayFromScrimmage() {}

public:
	static PlayFromScrimmage *getInstance();
	void enter(Game *game);
	/* Gets both teams playcalls and simulated a play. Depending on the outcome
	 * control may move into one of many states.
	 */
	void execute(Game *game);
	void exit(Game *game);
};

/* One team has just scored a touchdown. For the sake of my sanity, it is always
 * assumed that the team scoring will be stored in the Game's offense pointer,
 * so make sure you do this on turnovers!
 */
class Touchdown : public State<Game> {
private:
	Touchdown() {}

public:
	static Touchdown *getInstance();
	void enter(Game *game);
	/* Give six points to the offense and change state to ExtraPoint */
	void execute(Game *game);
	void exit(Game *game);
};

/* A game that has reached halftime and is ready to restart play.
 */
class Halftime : public State<Game> {
private:
	Halftime() {}

public:
	static Halftime *getInstance();
	void enter(Game *game);
	/* Set up for the away team to kickoff. */
	void execute(Game *game);
	void exit(Game *game);
};

/* A Game that has ended. Checking for this state is a good way to tell
 * whether the game is still going.
 */
class Final : public State<Game> {
private:
	Final() {}

public:
	static Final *getInstance();
	void enter(Game *game);
	void execute(Game *game);
	void exit(Game *game);
};

#endif
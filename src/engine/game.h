#ifndef __GAME_H
#define __GAME_H

#include "clock.h"
#include "states.h"
#include "team.h"
#include "utils.h"
#include <vector>

struct PlayOutcome;
class Team;

/**
 * An abstract object to be given the outcome of every play as soon as it
 * is calculated.
 */
class PlayByPlayObserver {
public:
    virtual void notify(PlayOutcome* outcome) = 0;
};

/**
 * An abstract object to be given the current Situation before every play.
 */
class SituationObserver {
public:
    virtual void onSituationChange(Situation* situation) = 0;
};

/**
 * set of possible game states. this is soon to be deprecated as we move to a
 * more sophisticated state machine model.
 */
enum Down { FIRST = 1,
    SECOND,
    THIRD,
    FOURTH,
    KICKOFF,
    PAT };

/**
 * Represents the state of the game at the start of a play.
 *
 * Implements PlayByPlayObserver, so you can register this object at the start
 * of the game and have the situation automatically updated at the end of
 * every play.
 *
 * Everything is public because this was a struct before I added the observer
 * functionality. Might encapsulate members later.
 */
class Situation : public PlayByPlayObserver {
public:
    Down down;
    int distance;
    int fieldPos;
    Clock* clock;

    Situation()
        : down(FIRST)
        , distance(10)
        , fieldPos(25)
    {
        clock = new Clock();
    }
    virtual ~Situation() { }

    /* Updates the down, distance, field position, and clock appropriately based
     * on the play outcome.
     */
    void notify(PlayOutcome* outcome);
};

/**
 * Keeps track of a team's stats during a game. Should be updated on each game
 * loop.
 *
 * TODO: register as a PlayByPlayObserver. Need to deal with deciding whether
 * the team is on offense or defense, though.
 */
struct TeamStats {
    int passingYards;
    int rushingYards;
    unsigned int passingPlays;
    unsigned int completions;
    unsigned int runningPlays;
    unsigned int sacks;
    unsigned int interceptions;
    unsigned int fumbles;

    int totalYards()
    {
        return passingYards + rushingYards;
    }
};

/*
 * Holds data associated to a team during a game, such as score and timeouts.
 *
 * I didn't bother with a destructor because both pointers here need a longer
 * lifespan that a TeamInfo object.
 */
struct TeamInfo {
    Team* team;
    // Question: should this be a part of TeamStats instead?
    unsigned int score;
    unsigned int timeouts;
    TeamStats* stats;

    /* Returns a team with 3 timeouts and no points */
    TeamInfo(Team* t)
    {
        team = t;
        score = 0;
        timeouts = 3;
        stats = new TeamStats();
    }
};

/**
 * Keeps track of all information within a single game of football. Its job is
 * to manage the main game loop and notify all observers of in game events.
 *
 * Implements the ClockListener interface, so that the state machine can be
 * updated at halftime, end of game, etc.
 *
 * The game flow is controlled via the StateMachine member. I am in the process of
 * moving as much logic out of Game and into either the state machine or the
 * various other member objects.
 */
class Game : public ClockListener {
private:
    /* The two teams playing. The offense and defense pointers always point to
     * either home or away.
     */
    TeamInfo* home;
    TeamInfo* away;
    TeamInfo* offense;
    TeamInfo* defense;

    /* Controls the state of the game. Call stateMachine.update() to run a play. */
    StateMachine<Game>* stateMachine;

    /*
     * All information about the game state at the time of a snap. Updated on
     * each run through the game loop.
     */
    Situation* situation;

    /* Observers to be delivered the outcome at the conclusion of each play.*/
    std::vector<PlayByPlayObserver*>* playObs;
    /* Observers to be given the situation before very snap. */
    std::vector<SituationObserver*>* sitObs;

    /* swaps the offense and defense pointers */
    void swapOffense();

public:
    /*
     * Sets up a game. Sets up the home team to start with the ball at their own
     * 25 yard line, because I haven't bothered with kickoffs yet.
     */
    Game(Team* homeTeam, Team* awayTeam);
    /* Frees up observer lists and situation objects. */
    virtual ~Game();
    /* Adds an observer to be given the outcome of every play */
    void registerPlayByPlayObs(PlayByPlayObserver* obs);
    /* Adds an observer to be given the situation before every snap */
    void registerSitObserver(SituationObserver* obs);
    /* The main game loop. Runs plays and handles changing situation based on
     * each outcome. Terminates when no time left in the 4th quarter.
     */
    void gameLoop();
    /* Changes possession, sets sitation to 1st and 10 at correct spot. */
    void changePossession();
    /* Gets both teams' playcalls and returns the play outcome. */
    PlayOutcome* callPlays() const;
    /* Update offensive/defensive stats with play outcome */
    void updateStats(PlayOutcome* outcome);

    /* Gets the current stats/scores for either team*/
    TeamStats* getHomeStats() const;
    TeamStats* getAwayStats() const;
    unsigned int getHomeScore() const;
    unsigned int getAwayScore() const;
    /* More getters */
    Situation* getSituation() const;
    StateMachine<Game>* getStateMachine() const;

    /* methods for notifiying these observers. */
    void notifySitObs();
    void notifyPlayByPlay(PlayOutcome* outcome);

    /* Setters */
    void giveOffensePoints(unsigned int points);
    void setHomePossession();
    void setAwayPossession();

    void updateDownAndDistance(PlayOutcome* outcome);
    void updateClock(PlayOutcome* outcome);

    /* Changes the game state based on clock events. Currently handles halftiem
     * and the end of the game.
     */
    void onClockEvent(AlarmType alarm);
};

#endif

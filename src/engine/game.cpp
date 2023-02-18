#include "game.h"
#include "clock.h"
#include "utils.h"
#include "gamestates.h"

/**
 * Prefix down incrementing. I feel that this is useful to have because downs
 * should increment in a logical and consistent way.
 *
 * Fun fact: A normal down is represented as the ordinal integer, so FIRST is 1.
 * This is not what I had before, and the change broke this briefly.
 */
Down operator++(Down &orig) {
	switch (orig) {
		case FIRST:
		case SECOND:
		case THIRD:
		case FOURTH:
			orig = static_cast<Down>((orig % 4) + 1);
			break;
		case KICKOFF:
			orig = FIRST;
			break;
		case PAT:
			orig = KICKOFF;
			break;
	}

	return orig;
}

/**
 * Postfix down incrementing, because if I only write one of these I will
 * inevitably use the other one.
 */
Down operator++(Down &orig, int) {
	Down tmp = orig;
	switch (orig) {
		case FIRST:
		case SECOND:
		case THIRD:
		case FOURTH:
			orig = static_cast<Down>((orig % 4) + 1);
			break;
		case KICKOFF:
			orig = FIRST;
			break;
		case PAT:
			orig = KICKOFF;
			break;
	}

	return tmp;
}

void Situation::notify(PlayOutcome *outcome) {
	fieldPos += outcome->yardsGained;
	distance -= outcome->yardsGained;

	if (distance <= 0) {
		// Again, need to send a message to the state machine
		down = FIRST;
		// goal to go situations...
		distance = MIN(10, 100 - fieldPos);
	} else if (down++ == FOURTH) {
		outcome->changePoss= true;
	}

	clock->runClock(outcome);
}

Game::Game(Team *homeTeam, Team *awayTeam) {

	home = new TeamInfo(homeTeam);
	away = new TeamInfo(awayTeam);
	offense = home;
	defense = away;

	situation = new Situation();
	situation->clock->setAlarm(this, HALFTIME);
	situation->clock->setAlarm(this, FINAL);

	playObs = new std::vector<PlayByPlayObserver *>();
	sitObs = new std::vector<SituationObserver *>();
	registerPlayByPlayObs(situation);

	stateMachine = new StateMachine<Game>(this);
	stateMachine->changeState(Kickoff::getInstance());
}

Game::~Game() {
	delete playObs;
	delete sitObs;
	delete situation->clock;
	delete situation;
	delete stateMachine;
}

TeamStats *Game::getHomeStats() const {
	return home->stats;
}

TeamStats *Game::getAwayStats() const {
	return away->stats;
}

unsigned int Game::getHomeScore() const {
	return home->score;
}

unsigned int Game::getAwayScore() const {
	return away->score;
}

StateMachine<Game> *Game::getStateMachine() const {
	return stateMachine;
}

Situation *Game::getSituation() const {
	return situation;
}

void Game::registerPlayByPlayObs(PlayByPlayObserver *obs) {
	playObs->push_back(obs);
}

void Game::registerSitObserver(SituationObserver *obs) {
	sitObs->push_back(obs);
}

void Game::swapOffense() {
	TeamInfo *tmp = offense;
	offense = defense;
	defense = tmp;
}

void Game::changePossession() {
	swapOffense();
	situation->fieldPos = 100 - situation->fieldPos;
	situation->down = FIRST;
	situation->distance = 10;
}

void Game::giveOffensePoints(unsigned int points) {
	offense->score += points;
}

void Game::setHomePossession() {
	offense = home;
	defense = away;
}

void Game::setAwayPossession() {
	offense = away;
	defense = home;
}

void Game::updateClock(PlayOutcome *outcome) {
	situation->clock->runClock(outcome);

	if (situation->clock->getTicks() <= 0) {
		situation->clock->advanceQuarter();
	}

	if (situation->clock->getQuarter() == 3) {
		stateMachine->changeState(Halftime::getInstance());
	} else if (situation->clock->getQuarter() == 5) {
		stateMachine->changeState(Final::getInstance());
	}
}

void Game::updateStats(PlayOutcome *outcome) {
	if (outcome->result == COMPLETED_PASS) {
		offense->stats->passingYards += outcome->yardsGained;
		offense->stats->completions++;
		offense->stats->passingPlays++;
	} else if (outcome->result == HANDOFF) {
		offense->stats->rushingYards += outcome->yardsGained;
		offense->stats->runningPlays++;
	} else if (outcome->result == SACK) {
		defense->stats->sacks++;
		offense->stats->passingYards += outcome->yardsGained;
	} else if (outcome->result == INTERCEPTION) {
		offense->stats->passingPlays++;
		defense->stats->interceptions++;
	} else if (outcome->result == INCOMPLETE_PASS) {
		offense->stats->passingPlays++;
	}
}

void Game::onClockEvent(AlarmType alarm) {
	switch (alarm) {
		case HALFTIME:
			stateMachine->changeState(Halftime::getInstance());
			break;
		case FINAL:
			stateMachine->changeState(Final::getInstance());
			break;
		case TWO_MIN_WARN:
			break;
	}
}

/**
 * Have the offense and defense call their plays, and then return the outcome of
 * the play.
 */
PlayOutcome *Game::callPlays() const {
	PlayCall offenseCall = offense->team->callPlay(situation);
	PlayCall defenseCall = defense->team->callPlay(situation);
	Play *play = new Play(offenseCall, defenseCall, situation);
	return play->runPlay();
}

void Game::updateDownAndDistance(PlayOutcome *outcome) {
		situation->fieldPos += outcome->yardsGained;
		situation->distance -= outcome->yardsGained;

		if (situation->distance <= 0) {
			situation->down = FIRST;
			// goal to go situations...
			situation->distance = MIN(10, 100 - situation->fieldPos);
		} else if (situation->down++ == FOURTH) {
			outcome->changePoss= true;
		}
}

void Game::notifySitObs() {
	std::vector<SituationObserver *>::iterator it = sitObs->begin();
	while (it != sitObs->end()) {
		(*it)->onSituationChange(situation);
		it++;
	}
}

void Game::notifyPlayByPlay(PlayOutcome *outcome) {
	std::vector<PlayByPlayObserver *>::iterator it = playObs->begin();
	while (it != playObs->end()) {
		(*it)->notify(outcome);
		it++;
	}
}

void Game::gameLoop() {
	while (!stateMachine->inState(*(Final::getInstance())))
		stateMachine->update();
}

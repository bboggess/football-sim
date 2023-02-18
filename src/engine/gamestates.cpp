#include "gamestates.h"

Kickoff *Kickoff::getInstance() {
	static Kickoff instance;

	return &instance;
}

void Kickoff::enter(Game *game) {
	Situation *sit = game->getSituation();

	sit->fieldPos = 35;
	sit->down = KICKOFF;
}

void Kickoff::execute(Game *game) {
	// TODO: implement kickoff logic
	game->changePossession();
	game->getStateMachine()->changeState(PlayFromScrimmage::getInstance());
}

void Kickoff::exit(Game *game) {
	game->getSituation()->fieldPos = 25;
}

void PlayFromScrimmage::enter(Game *game) {

}

void PlayFromScrimmage::execute(Game *game) {
	game->notifySitObs();

	PlayOutcome *outcome = game->callPlays();
	game->notifyPlayByPlay(outcome);
	game->updateStats(outcome);

	if (outcome->changePoss)
		game->changePossession();

	if (outcome->touchdown) {
		game->getStateMachine()->changeState(Touchdown::getInstance());
	} else if (outcome->result == FIELD_GOAL_MADE) {
		game->giveOffensePoints(3);
		game->getStateMachine()->changeState(Kickoff::getInstance());
	}
}

void PlayFromScrimmage::exit(Game *game) {

}


PlayFromScrimmage *PlayFromScrimmage::getInstance() {
	static PlayFromScrimmage instance;

	return &instance;
}

Touchdown *Touchdown::getInstance() {
	static Touchdown instance;

	return &instance;
}

void Touchdown::enter(Game *game) {

}

void Touchdown::execute(Game *game) {
	game->giveOffensePoints(7);
	game->getStateMachine()->changeState(ExtraPoint::getInstance());
}

void Touchdown::exit(Game *game) {
}

ExtraPoint *ExtraPoint::getInstance() {
	static ExtraPoint instance;

	return &instance;
}

void ExtraPoint::enter(Game *game) {
	Situation *sit = game->getSituation();

	sit->fieldPos = 97;
	sit->down = PAT;
}

void ExtraPoint::execute(Game *game) {
	game->getStateMachine()->changeState(Kickoff::getInstance());
}

void ExtraPoint::exit(Game *game) {

}

Halftime *Halftime::getInstance() {
	static Halftime instance;

	return &instance;
}

void Halftime::enter(Game *game) {
	
}

void Halftime::execute(Game *game) {
	game->setAwayPossession();
	game->getStateMachine()->changeState(Kickoff::getInstance());
}

void Halftime::exit(Game *game) {

}

Final *Final::getInstance() {
	static Final instance;

	return &instance;
}

void Final::enter(Game *game) {
	
}

void Final::execute(Game *game) {

}

void Final::exit(Game *game) {

}

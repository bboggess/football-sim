/**
 * main.cpp
 *
 * Basically a driver for testing the simulation engine. I'll remove this from
 * this folder once I feel like this can work as a standalone library.
 */

#include <iostream>
#include <map>
#include <string>
#include <cstdlib>
#include <ctime>

#include "engine/game.h"
#include "engine/playcall.h"
#include "engine/team.h"
#include "learn/model.h"

/*
 * Yes, I stole these from StackExchange. Why is it so goddamn complicated to
 * instantiate a simple map? I already know all of the possible values.
 */
template<typename T>
struct map_init_helper
{
    T& data;
    map_init_helper(T& d) : data(d) {}
    map_init_helper& operator() (typename T::key_type const& key, typename T::mapped_type const& value)
    {
        data[key] = value;
        return *this;
    }
};

template<typename T>
map_init_helper<T> map_init(T& item)
{
    return map_init_helper<T>(item);
}

/*
 * Prints the outcome of each play.
 */
class Commentator : public PlayByPlayObserver {
	std::map<PlayResult, const char *> results;

	public:
		Commentator() {
			map_init(results)
				(COMPLETED_PASS, "Pass completed")
				(INCOMPLETE_PASS, "Pass incomplete")
				(FUMBLE, "Fumble")
				(INTERCEPTION, "Interception")
				(HANDOFF, "Run")
				(SACK, "Sacked")
				(FIELD_GOAL_MADE, "Field goal is good!")
				(FIELD_GOAL_MISS, "Field goal is missed")
				(PUNT_RETURN, "Punt");
		}

		void notify(PlayOutcome *outcome);
};

/*
 * Prints down, distance, yardline and time before each play.
 */
class ScoreboardOp : public SituationObserver {
	std::map<Down, const char *> downs;

public:
	ScoreboardOp() {
		map_init(downs)
			(FIRST, "First down")
			(SECOND, "Second down")
			(THIRD, "Third down")
			(FOURTH, "Fourth down");
	}

	void onSituationChange(Situation *sit);
};

void Commentator::notify(PlayOutcome *outcome) {
	if (outcome->touchdown)
		std::cout << "TOUCHDOWN! ";
	if (outcome->changePoss)
		std::cout << "TURNOVER! ";
	std::cout << results[outcome->result];
	std::cout << " for ";
	std::cout << outcome->yardsGained;
	std::cout << " yards\n";
}

void ScoreboardOp::onSituationChange(Situation *sit) {
	std::cout << downs[sit->down];
	std::cout << " and ";
	std::cout << ((sit->fieldPos + sit->distance >= 100) ? "goal" : std::to_string(sit->distance));
	std::cout << " from the ";
	std::cout << sit->fieldPos;
	std::cout << " yard line\n";
	std::cout << sit->clock->ticksToTime() + " remaining in quarter ";
	std::cout << sit->clock->getQuarter() << "\n";
}

void printScore(double home, double away) {
	std::cout << home << "-" << away << '\n';
}

double average(unsigned int scores[], size_t len) {
	unsigned int total = 0;
	for (size_t i = 0; i < len; ++i)
		total += scores[i];

	return static_cast<double>(total) / static_cast<double>(len);
}

TeamStats *averageStats(TeamStats *stats[], size_t len) {
	TeamStats *avg = new TeamStats();
	int passAvg = 0;
	int rushAvg = 0;
	int attempts = 0;
	int completions = 0;
	int rushes = 0;
	for (size_t i = 0; i < len; i++) {
		passAvg += stats[i]->passingYards;
		rushAvg += stats[i]->rushingYards;
		attempts += stats[i]->passingPlays;
		completions += stats[i]->completions;
		rushes += stats[i]->runningPlays;
	}

	avg->passingYards = passAvg / len;
	avg->rushingYards = rushAvg / len;
	avg->passingPlays = attempts / len;
	avg->runningPlays = rushes / len;
	avg->completions = completions / len;

	return avg;
}

/*
 * Run 1000 games and tell me the average score and stats.
 */
int main() {
	srand (time(0));
	initModel();
	Commentator *commentator = new Commentator();
	ScoreboardOp *op = new ScoreboardOp();
	size_t numTrials = 1;
	unsigned int homeScores[numTrials];
	unsigned int awayScores[numTrials];
	TeamStats *homeStats[numTrials];
	TeamStats *awayStats[numTrials];
	for (size_t i = 0; i < numTrials; i++) {
		Team *home = new AITeam();
		Team *away = new AITeam();
		Game *game = new Game(home, away);
		game->registerPlayByPlayObs(commentator);
		game->registerSitObserver(op);
		game->gameLoop();
		homeScores[i] = game->getHomeScore();
		awayScores[i] = game->getAwayScore();
		homeStats[i] = game->getHomeStats();
		awayStats[i] = game->getAwayStats();
		delete game;
		delete home;
		delete away;
	}

	printScore(average(homeScores, numTrials), average(awayScores, numTrials));
	TeamStats *homeavg = averageStats(homeStats, numTrials);
	TeamStats *awayavg = averageStats(awayStats, numTrials);
	std:: cout << "Passing Yards: ";
	printScore(homeavg->passingYards, awayavg->passingYards);
	std::cout << "Passing Attempts: ";
	printScore(homeavg->passingPlays, awayavg->passingPlays);
	std::cout << "Completions: ";
	printScore(homeavg->completions, awayavg->completions);
	std::cout << "Rushing Yards: ";
	printScore(homeavg->rushingYards, awayavg->rushingYards);
	std::cout << "Rushing Attempts:";
	printScore(homeavg->runningPlays, awayavg->runningPlays);
}

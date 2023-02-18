#include <iostream>

#include "team.h"

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

#define NUM_PLAYS 5
PlayCall plays[] = { RUN, SHORT_PASS, LONG_PASS, PUNT, FIELD_GOAL };
std::map<PlayCall, const char *> playNames;
std::map<Down, const char *> downs;

UserTeam::UserTeam() {
	map_init(playNames)
		(RUN, "Run")
		(SHORT_PASS, "Short pass")
		(LONG_PASS, "Long pass")
		(PUNT, "Punt")
		(FIELD_GOAL, "Field goal");

	map_init(downs)
		(FIRST, "First down")
		(SECOND, "Second down")
		(THIRD, "Third down")
		(FOURTH, "Fourth down");
}

void printSituation(Situation *sit) {
	std::cout << downs[sit->down];
	std::cout << " and ";
	std::cout << ((sit->fieldPos + sit->distance >= 100) ? " and goal" : std::to_string(sit->distance));
	std::cout << " from the ";
	std::cout << sit->fieldPos;
	std::cout << " yard line\n";
	std::cout << sit->clock->ticksToTime() + " remaining in quarter ";
	std::cout << sit->clock->getQuarter() << "\n";
}

void printPrompt() {
	std::cout << "Pick your play from the following" << std::endl;

	for (size_t i = 0; i < NUM_PLAYS; i++)
		std::cout << i << ": " << playNames[plays[i]] << std::endl;
}

void printError() {
	std::cout << "Sorry, did not recognize that option." << std::endl;
}

PlayCall UserTeam::callPlay(Situation *sit) {
	unsigned int in;

	printSituation(sit);
	while (1) {
		printPrompt();
		std::cin >> in;
		if (in < NUM_PLAYS)
			break;
		else
			printError();
	}

	return plays[in];
}
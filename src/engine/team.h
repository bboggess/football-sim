#ifndef __TEAM_H
#define __TEAM_H

#include "playcall.h"
#include "game.h"

/**
 * Should be the base Team class. Currently only responsible for calling plays,
 * but the plan is for each team to have its own playcalling style, strengths,
 * weaknesses, whatever.
 *
 * The user of this engine is free to extend this class in any way necessary. I
 * anticipate that any situations in which the user would like to call their own
 * plays will be implemented by extending Team.
 */
class Team {
public:
	virtual ~Team() {};
	virtual PlayCall callPlay(Situation *situation) =0;
};

/**
 * Defines the callPlay function by using the machine learning model present in
 * the data directory. Basically, most actual teams being implemented should
 * probably just extend this basic AI class for basic, somewhat intelligent,
 * playcalling capabilities.
 */
class AITeam : public Team {
public:
	PlayCall callPlay(Situation *situation);
};

#include <map>

class UserTeam : public Team {
public:
	UserTeam();
	PlayCall callPlay(Situation *situation);
};

#endif

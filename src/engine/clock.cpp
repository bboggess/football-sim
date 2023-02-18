#include "clock.h"
#include "playcall.h"
#include <string>

Clock::Clock()
{
    setQuarter(1);
    ticks = QUARTER_LEN;
    alarms = new std::map<AlarmType, std::vector<ClockListener*>*>();
    (*alarms)[HALFTIME] = new std::vector<ClockListener*>();
    (*alarms)[FINAL] = new std::vector<ClockListener*>();
    (*alarms)[TWO_MIN_WARN] = new std::vector<ClockListener*>();
}

Clock::~Clock()
{
    delete (*alarms)[HALFTIME];
    delete (*alarms)[FINAL];
    delete (*alarms)[TWO_MIN_WARN];
    delete alarms;
}

unsigned int Clock::getTicks()
{
    return ticks >= 0 ? ticks : 0;
}

int Clock::setQuarter(unsigned int n)
{
    if (n < 1 || n > 4)
        return -1;

    quarter = n;
    return n;
}

unsigned int Clock::getQuarter()
{
    return quarter;
}

unsigned int Clock::getMinutes()
{
    return getTicks() * SECONDS_PER_TICK / 60;
}

unsigned int Clock::getSeconds()
{
    return (getTicks() * SECONDS_PER_TICK) % 60;
}

void Clock::notifyHalftime()
{
    std::vector<ClockListener*>* halftimeList = (*alarms)[HALFTIME];
    std::vector<ClockListener*>::iterator it = halftimeList->begin();

    while (it != halftimeList->end()) {
        (*it)->onClockEvent(HALFTIME);
        it++;
    }
}

void Clock::notifyFinal()
{
    std::vector<ClockListener*>* finalList = (*alarms)[FINAL];
    std::vector<ClockListener*>::iterator it = finalList->begin();

    while (it != finalList->end()) {
        (*it)->onClockEvent(FINAL);
        it++;
    }
}

unsigned int Clock::advanceQuarter()
{
    if (quarter == 2)
        notifyHalftime();
    else if (quarter == 4)
        notifyFinal();

    setQuarter(quarter + 1);
    ticks = QUARTER_LEN;

    return quarter;
}

int Clock::runClock(PlayOutcome* outcome)
{
    unsigned int runoff = 0;
    switch (outcome->result) {
    case HANDOFF:
    case COMPLETED_PASS:
    case SACK:
        runoff = 4;
        break;
    case INCOMPLETE_PASS:
    case KICK_RETURN:
    case FIELD_GOAL_MADE:
    case FIELD_GOAL_MISS:
    case FIELD_GOAL_BLOCK:
    case INTERCEPTION:
    case FUMBLE:
        runoff = 1;
        break;
    default:
        runoff = 0;
        break;
    }

    if (outcome->touchdown)
        runoff = 1;

    ticks -= runoff;

    if (ticks <= 0)
        advanceQuarter();

    return runoff;
}

void Clock::setAlarm(ClockListener* listener, AlarmType alarm)
{
    (*alarms)[alarm]->push_back(listener);
}

std::string Clock::ticksToTime()
{
    const char* format = "%M:%S";

    unsigned int minutes = getMinutes();
    unsigned int seconds = getSeconds();

    struct tm* time = new tm();
    time->tm_sec = seconds;
    time->tm_min = minutes;

    char toReturn[6];
    strftime(toReturn, 6, format, time);
    delete time;
    return toReturn;
}
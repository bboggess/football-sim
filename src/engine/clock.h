#ifndef __CLOCK_H
#define __CLOCK_H

#include "playcall.h"
#include <map>
#include <string>
#include <vector>

// num of ticks per quarter
const unsigned QUARTER_LEN = 90;

// converts from internal representation of time to real seconds
const unsigned SECONDS_PER_TICK = 15 * 60 / QUARTER_LEN;

/* Used for ClockListeners. A ClockListener can ask to be notified when any of
 * these events occurs.
 *
 * HALFTIME: when the second quarter ends
 * FINAL: when the fourth quarter ends
 * TWO_MIN_WARN: the two minute warning in the second and fourth quarters
 */
enum AlarmType { HALFTIME,
    FINAL,
    TWO_MIN_WARN };

/**
 * An interface for setting an alarm with a Clock object
 */
class ClockListener {
public:
    /* After a call to Clock::setAlarm(), this method will be called at the
     * requested point in time.
     *
     * The type of alarm that has been triggered is passed, allowing listeners
     * to set alarms for events of different types.
     */
    virtual void onClockEvent(AlarmType alarm) = 0;
};

/**
 * Clock object keeping time in a game. Initializes to 1st quarter with 15
 * minutes remaining, and contains logic for running clock after plays.
 */
class Clock {
private:
    /* Quarter options. Currently must be in [1,4], i.e. no OT support. */
    unsigned int quarter;
    /* Game time is represented internally by ticks. These can be converted
     * into seconds using the SECONDS_PER_TICK macro
     */
    int ticks;
    /* Sets the quarter. If n is not in [1,4], returns -1 and does not change
     * the quarter field.
     */
    int setQuarter(unsigned int n);
    /* For each alarm type, has a list of listeners to receive a notification.*/
    std::map<AlarmType, std::vector<ClockListener*>*>* alarms;
    /* Notifies all observers who set an alarm for halftime */
    void notifyHalftime();
    /* Notifies observers who set an alarm for the end of the game. */
    void notifyFinal();

public:
    /* Initializes a new clock to 15:00 remaining in the 1st quarter. */
    Clock();
    ~Clock();
    /* Runs appropriate amount of time off of the clock depending on the
     * play outcome.
     *
     * I actually want to set Clock as a PlayByPlayListener and have this be
     * the subscription method at some point.
     */
    int runClock(PlayOutcome* outcome);
    /* Returns the remaining number of ticks, which is guaranteed to be
     * nonnegative.
     */
    unsigned int getTicks();
    /* Returns the time remaining in the quarter as a string of the form
     * MM:SS
     */
    std::string ticksToTime();
    /* Advances quarter and sets time reamining to 15:00 . */
    unsigned int advanceQuarter();
    /* Returns the current quarter. */
    unsigned int getQuarter();
    /* Returns the number of minutes left in the quarter. E.g. if there is
     * 5:30 left in the 2nd, returns 5
     */
    unsigned int getMinutes();
    /* Returns the number of minutes left in the current minute. E.g. if
     * there is  5:30 left in the 2nd, returns 30
     */
    unsigned int getSeconds();
    /* Requests the listener's onCloclEvent() be called when events of type
     * alarm occur
     */
    void setAlarm(ClockListener* listener, AlarmType alarm);
};

#endif
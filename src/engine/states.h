#ifndef __STATES_H
#define __STATES_H

#include <assert.h>
#include <typeinfo>

/* An abstract state to be used in a finite state machine model.
 * Defines a simple interface for executing in a given state, and cleanup
 * functions when switching states.
 */
template <class Entity>
class State {
public:
    virtual ~State() { }

    /* Called when the current state first switches to this, before execute is
     * called
     */
    virtual void enter(Entity*) = 0;
    /* Implements the main behavior the entity should carry out while in this
     * state.
     */
    virtual void execute(Entity*) = 0;
    /* Called when the state is changed away from this, before the enter
     * function of the new state.
     */
    virtual void exit(Entity*) = 0;
};

/* Controller for an abstract state. Keeps track of the state of some entity,
 * with all logic for executing and changing states.
 */
template <class Entity>
class StateMachine {
private:
    Entity* owner;
    State<Entity>* curState;

public:
    /* Note: curState is NULL after a call to the constructor! */
    StateMachine(Entity* own)
        : owner(own)
        , curState(nullptr)
    {
    }

    /* Calls the current state's execute function. */
    void update() const
    {
        if (curState)
            curState->execute(owner);
    }

    /* Changes state to newState, performing any necessary cleanup on both the
     * current and new state. Make sure that newState is NOT null
     */
    void changeState(State<Entity>* newState)
    {
        assert(newState && "[StateMachine::changeState] Trying to change to a null state");

        if (curState)
            curState->exit(owner);
        curState = newState;
        curState->enter(owner);
    }

    /* getters... */
    State<Entity>* getCurrentState() const { return curState; }
    Entity* getOwner() const { return owner; }

    /* Checks whether the entity is currently in the given state. Basically
     * equivalent to calling typeid(*getCurrentState()) == typeid(state),
     * just a bit more brainless.
     */
    bool inState(const State<Entity>& state) const
    {
        return typeid(*curState) == typeid(state);
    }
};

#endif
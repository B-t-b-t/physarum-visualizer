#ifndef OBSERVER_H
#define OBSERVER_H

#include "event.h"

class Observable;   //forward declaration to avoid circular dependency

class Observer {

friend class Observable;  //to allow Observable to remove itself when destroyed

public:
    Observer() = default;
    virtual ~Observer();

    virtual void onNotify(const Event event) = 0;
    
protected:
    void setObservable(Observable* observable);
    void resetObservable() { observable_ = nullptr; }   //exists to simplify nullptr check in setObservable

    Observable* observable_{nullptr};
};

#endif // OBSERVER_H
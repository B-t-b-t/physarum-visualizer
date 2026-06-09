#ifndef OBSERVER_H
#define OBSERVER_H

#include "event.h"

class Observable;   //forward declaration to avoid circular dependency

class Observer {
public:
    Observer() = default;
    Observer(Event event, Observable* observable);
    ~Observer();

    virtual void onNotify(const Event event) = 0;
    void attachToObservable(Event event, Observable* observable);

protected:
    Observable* observable_{nullptr};
};

#endif // OBSERVER_H
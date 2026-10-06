#ifndef OBSERVER_H
#define OBSERVER_H

#include <list>

class Observable;   //forward declaration to avoid circular dependency
struct UserEvent;      //forward declaration to avoid circular dependency

class Observer {

friend class Observable;  //to allow Observable to remove itself when destroyed

public:
    Observer() = default;
    virtual ~Observer();

    virtual void onNotify(const UserEvent event) = 0;
    
protected:
    void addObservable(Observable* observable);
    void resetObservable() { observable_.clear(); }
    void removeObservable(Observable* observable) { observable_.remove(observable); }

    std::list<Observable*> observable_;     //this list just exists to inform all Observables, if this Observer is destroyed
};

#endif // OBSERVER_H
#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <list>
#include <unordered_map>

#include "event.h"

class Observer;  //forward declaration to avoid circular dependency

class Observable {
    public: 

    virtual ~Observable();
    
    void notify(const Event event);
    void notifyAll();
    
    void addObserver(Event event, Observer* observer);
    void removeObserver(Event event, Observer* observer);
    void removeObserverAll(Observer* observer);
    
    protected:
    
    typedef std::list<Observer*> ObserverList;
    std::unordered_map<Event, ObserverList> observers_{};
};

#endif // OBSERVABLE_H
#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <list>
#include <unordered_map>

class Observer;  //forward declaration to avoid circular dependency
struct UserEvent; //forward declaration to avoid circular dependency
enum class EventType; //forward declaration to avoid circular dependency

class Observable {
    public: 

    virtual ~Observable();
    
    void notify(const UserEvent event);
    
    void addObserver(EventType event, Observer* observer);
    void removeObserver(EventType event, Observer* observer);
    void removeObserverAll(Observer* observer);
    
    protected:
    
    typedef std::list<Observer*> ObserverList;
    std::unordered_map<EventType, ObserverList> observers_{};
};

#endif // OBSERVABLE_H
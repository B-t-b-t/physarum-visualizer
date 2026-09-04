#include "observable.h"

#include <utility>

#include "event.h"
#include "observer.h"

Observable::~Observable() {
    for(auto& pair : observers_) {
        ObserverList& obsList = pair.second;
        for(Observer* observer : obsList) {
            observer->resetObservable();
        }
    }
}

void Observable::notify(const UserEvent event) {
    ObserverList& obsList = observers_[event.type];
    for(Observer* observer : obsList) {
        observer->onNotify(event);
    }
}

void Observable::addObserver(EventType event, Observer* observer) {
    if(observer) {
        ObserverList& obsList = observers_[event];
        obsList.push_back(observer);
        observer->setObservable(this);
    }
}

void Observable::removeObserver(EventType event, Observer* observer) {
    if(observer) {
        ObserverList& obsList = observers_[event];
        obsList.remove(observer);
    }
}

void Observable::removeObserverAll(Observer* observer) {
    if(observer) {
        for(auto& pair : observers_) {
            ObserverList& obsList = pair.second;
            obsList.remove(observer);
        }
    }
}

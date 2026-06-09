#include "observable.h"

#include "observer.h"

void Observable::notify(const Event event) {
    ObserverList& obsList = observers_[event];
    for(Observer* observer : obsList) {
        observer->onNotify(event);
    }
}

void Observable::notifyAll() {
    for(const auto& pair : observers_) {
        Event event = pair.first;
        notify(event);
    }
}

void Observable::addObserver(Event event, Observer* observer) {
    if(observer) {
        ObserverList& obsList = observers_[event];
        obsList.push_back(observer);
    }
}

void Observable::removeObserver(Event event, Observer* observer) {
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

#include "observable.h"

#include <algorithm>
#include <utility>

#include "event.h"
#include "observer.h"

Observable::~Observable() {
    for(auto& pair : observers_) {
        ObserverList& obsList = pair.second;
        for(Observer* observer : obsList) {
            observer->removeObservable(this);
        }
    }
}

void Observable::notify(const UserEvent event) {
    //iterate over a snapshot, so callbacks may safely add/remove observers.
    const ObserverList snapshot = observers_[event.type];
    for(Observer* observer : snapshot) {
        //skip observers that were removed by an earlier callback.
        const ObserverList& current = observers_[event.type];
        if(std::find(current.begin(), current.end(), observer) != current.end()) {
            observer->onNotify(event);
        }
    }
}

void Observable::addObserver(EventType event, Observer* observer) {
    if(observer) {
        ObserverList& obsList = observers_[event];
        if(std::find(obsList.begin(), obsList.end(), observer) == obsList.end()) {
            obsList.push_back(observer);
        }
        observer->addObservable(this);
    }
}

void Observable::removeObserver(EventType event, Observer* observer) {
    if(observer) {
        ObserverList& obsList = observers_[event];
        obsList.remove(observer);
        if(!isObservedBy(observer)) {
            observer->removeObservable(this);
        }
    }
}

void Observable::removeObserverAll(Observer* observer) {
    if(observer) {
        for(auto& pair : observers_) {
            ObserverList& obsList = pair.second;
            obsList.remove(observer);
        }
        observer->removeObservable(this);
    }
}

bool Observable::isObservedBy(const Observer* observer) const {
    for(const auto& pair : observers_) {
        const ObserverList& obsList = pair.second;
        if(std::find(obsList.begin(), obsList.end(), observer) != obsList.end()) {
            return true;
        }
    }
    return false;
}

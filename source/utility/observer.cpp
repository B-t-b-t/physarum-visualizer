#include "observer.h"

#include "observable.h"

Observer::Observer(Event event, Observable* observable) :observable_{observable} {
    if (observable_) {
        observable_->addObserver(event, this);
    }
}

Observer::~Observer() {
    if (observable_) {
        observable_->removeObserverAll(this);
    }
}

void Observer::attachToObservable(Event event, Observable* observable) {
    if (!observable_ && observable) {
        observable_ = observable;
        observable_->addObserver(event, this);
    }
}
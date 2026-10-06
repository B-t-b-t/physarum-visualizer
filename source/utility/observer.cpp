#include "observer.h"

#include <algorithm>
#include <cassert>

#include "observable.h"

Observer::~Observer() {
    for (auto* obs : observable_) {
        obs->removeObserverAll(this);
    }
}

void Observer::addObservable(Observable* observable) {
    assert((observable != nullptr) && "Observable is NULL");

    if(observable != nullptr) {
        observable_.push_back(observable);
    }
}
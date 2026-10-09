#include "observer.h"

#include <algorithm>
#include <cassert>

#include "observable.h"

Observer::~Observer() {
    // removeObserverAll() edits observable_, so work on a copy.
    const std::list<Observable*> observables = observable_;
    for (auto* obs : observables) {
        obs->removeObserverAll(this);
    }
}

void Observer::addObservable(Observable* observable) {
    assert((observable != nullptr) && "Observable is NULL");

    //avoid adding the same observable multiple times
    if(observable != nullptr
        && std::find(observable_.begin(), observable_.end(), observable) == observable_.end()) {
        observable_.push_back(observable);
    }
}
#include "observer.h"

#include <cassert>

#include "observable.h"

Observer::~Observer() {
    if (observable_) {
        observable_->removeObserverAll(this);
    }
}

void Observer::setObservable(Observable* observable) {
    assert((observable_ == nullptr || (observable_ != nullptr && observable_ == observable)) && "Observer is already associated with a different Observable.");
    
    if(observable_ == nullptr || (observable_ != nullptr && observable_ == observable)) {
        observable_ = observable; 
    }
}
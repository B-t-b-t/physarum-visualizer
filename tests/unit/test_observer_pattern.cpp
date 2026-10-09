#include <catch2/catch_test_macros.hpp>        // for operator""_catch_sr

#include <cstddef>                             // for size_t
#include <functional>
#include <memory>
#include <list>                                // for list, _List_const_iter...
#include <optional>                            // for optional
#include <string>                              // for basic_string, operator==
#include <unordered_map>                       // for unordered_map
#include <utility>                             // for get
#include <variant>                             // for get
#include <vector>                              // for vector

#include "simulation/trail_mask_properties.h"  // for TrailMaskProperties
#include "utility/event.h"                     // for UserEvent, EventType
#include "utility/observable.h"                // for Observable
#include "utility/observer.h"                  // for Observer

class RecordingObserver final : public Observer {
public:
    void onNotify(const UserEvent event) override {
        allReceivedEvents.push_back(event);
    }

    size_t getObservableCount() const { return observable_.size(); }

    Observable* getFirstObservable() const { return observable_.empty() ? nullptr : *observable_.begin(); }

    std::vector<UserEvent> allReceivedEvents;
};

class TestObservable final : public Observable {
public:
    void publish(const UserEvent& event) {
        notify(event);
    }
    size_t getRegisteredEventCount() const {
        return observers_.size();
    }

    size_t getObserverCount() const {
        size_t count = 0;

        for (const auto& [type, observers] : observers_) {
            count += observers.size();
        }
        return count;
    }

    size_t getObserverCountForEvent(EventType type) const {
        return observers_.at(type).size();
    }
};

TEST_CASE("Observable notifies observers registered for an event", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, std::string{"preset.psf"}});

    REQUIRE(observer.allReceivedEvents.size() == 1);
    CHECK(observer.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_CREATE);
    CHECK(std::get<std::string>(observer.allReceivedEvents[0].payload) == "preset.psf");
}

TEST_CASE("Observable only notifies observers subscribed to the matching event", "[observer]") {
    TestObservable observable;
    RecordingObserver saveObserver;
    RecordingObserver loadObserver;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &saveObserver);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &loadObserver);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 1});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 2});

    REQUIRE(saveObserver.allReceivedEvents.size() == 1);
    REQUIRE(loadObserver.allReceivedEvents.size() == 1);
    CHECK(saveObserver.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_CREATE);
    CHECK(loadObserver.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_APPLY);
}

TEST_CASE("An observer can subscribe to multiple events", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0});

    REQUIRE(observer.allReceivedEvents.size() == 2);
    CHECK(observer.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_CREATE);
    CHECK(observer.allReceivedEvents[1].type == EventType::BEHAVIOR_PRESET_APPLY);
}

TEST_CASE("Observable removes an observer from one event without affecting other events", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);

    observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0});

    REQUIRE(observer.allReceivedEvents.size() == 1);
    CHECK(observer.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_APPLY);
}

TEST_CASE("Observable removes an observer from all event subscriptions", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);
    observable.removeObserverAll(&observer);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0});

    CHECK(observer.allReceivedEvents.empty());
}

TEST_CASE("Observable safely handles missing subscriptions and null observers", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, nullptr);
    observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, nullptr);
    observable.removeObserverAll(nullptr);

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0});

    CHECK(observer.allReceivedEvents.empty());
}

TEST_CASE("Observer destruction unsubscribes it from its observable", "[observer]") {
    TestObservable observable;
    RecordingObserver remainingObserver;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &remainingObserver);

    {
        RecordingObserver temporaryObserver;
        observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &temporaryObserver);
        RecordingObserver temporaryObserver2;
        observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &temporaryObserver2);
    }

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    REQUIRE(remainingObserver.allReceivedEvents.size() == 1);
    REQUIRE(observable.getRegisteredEventCount() == 2);
    REQUIRE(observable.getObserverCount() == 1);
    REQUIRE(observable.getObserverCountForEvent(EventType::BEHAVIOR_PRESET_CREATE) == 1);
    REQUIRE(observable.getObserverCountForEvent(EventType::BEHAVIOR_PRESET_APPLY) == 0);
}

TEST_CASE("Observable destruction allows an observer to be attached again", "[observer]") {
    RecordingObserver observer;

    {
        TestObservable firstObservable;
        firstObservable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
        firstObservable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
        REQUIRE(observer.getFirstObservable() == &firstObservable);
    }

    //test sucessfull reset after observable destruction
    REQUIRE(observer.getFirstObservable() == nullptr);

    TestObservable secondObservable;
    secondObservable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    secondObservable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    REQUIRE(observer.getFirstObservable() == &secondObservable);

    REQUIRE(observer.allReceivedEvents.size() == 2);
}

TEST_CASE("Observable preserves complex event payloads", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    TrailMaskProperties props{};
    props.strength = 2.0;
    props.text = "night-mask";

    const TrailMaskData mask{
        "night-mask",
        TrailMaskType::TEXT,
        props
    };

    observable.addObserver(EventType::TEXT_PRESET_EDIT, &observer);
    observable.publish({EventType::TEXT_PRESET_EDIT, mask});

    REQUIRE(observer.allReceivedEvents.size() == 1);

    const auto& receivedMask = std::get<TrailMaskData>(observer.allReceivedEvents[0].payload);
    CHECK(receivedMask.name == "night-mask");
    CHECK(receivedMask.type == TrailMaskType::TEXT);
    CHECK_FALSE(receivedMask.properties.timeSlot);
    CHECK(receivedMask.properties.strength == 2.0);
}
// ---------------------------------------------------------------------------
// Multiple observables per observer
// ---------------------------------------------------------------------------

TEST_CASE("An observer can observe multiple observables", "[observer][multi]") {
    TestObservable first;
    TestObservable second;
    RecordingObserver observer;

    first.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    second.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);

    first.publish({EventType::BEHAVIOR_PRESET_CREATE, 1});
    second.publish({EventType::BEHAVIOR_PRESET_CREATE, 2});

    CHECK(observer.getObservableCount() == 2);
    REQUIRE(observer.allReceivedEvents.size() == 2);
    CHECK(std::get<int>(observer.allReceivedEvents[0].payload) == 1);
    CHECK(std::get<int>(observer.allReceivedEvents[1].payload) == 2);
}

TEST_CASE("Subscribing to several events of one observable registers it only once", "[observer][multi]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);
    observable.addObserver(EventType::COLOR_PRESET_APPLY, &observer);

    CHECK(observer.getObservableCount() == 1);
}

TEST_CASE("Adding the same observer twice for one event notifies it only once", "[observer][multi]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(observer.allReceivedEvents.size() == 1);
    CHECK(observable.getObserverCountForEvent(EventType::BEHAVIOR_PRESET_CREATE) == 1);
    CHECK(observer.getObservableCount() == 1);
}

TEST_CASE("Destroying one observable keeps the observer attached to the others", "[observer][multi]") {
    RecordingObserver observer;
    TestObservable survivor;
    survivor.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);

    {
        TestObservable temporary;
        temporary.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
        REQUIRE(observer.getObservableCount() == 2);
    }

    CHECK(observer.getObservableCount() == 1);
    CHECK(observer.getFirstObservable() == &survivor);

    survivor.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    CHECK(observer.allReceivedEvents.size() == 1);
}

TEST_CASE("Destroying an observer unsubscribes it from all its observables", "[observer][multi]") {
    TestObservable first;
    TestObservable second;

    {
        RecordingObserver observer;
        first.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
        second.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);
    }

    CHECK(first.getObserverCount() == 0);
    CHECK(second.getObserverCount() == 0);
    // Must not call a dangling observer.
    first.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    second.publish({EventType::BEHAVIOR_PRESET_APPLY, 0});
}

TEST_CASE("removeObserverAll on one observable keeps other observables attached", "[observer][multi]") {
    TestObservable first;
    TestObservable second;
    RecordingObserver observer;

    first.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    second.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    first.removeObserverAll(&observer);

    first.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    second.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(observer.allReceivedEvents.size() == 1);
    CHECK(observer.getObservableCount() == 1);
    CHECK(observer.getFirstObservable() == &second);
}

TEST_CASE("Unsubscribing keeps the observer's observable list in sync (no dangling pointer)", "[observer][multi]") {
    RecordingObserver observer;

    {
        TestObservable observable;
        observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
        observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
        CHECK(observer.getObservableCount() == 0);

        observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
        observable.removeObserverAll(&observer);
        CHECK(observer.getObservableCount() == 0);
    }
    // The observable is gone; destroying the observer must not touch it (checked by sanitizers).
}

TEST_CASE("Partial unsubscribe keeps the observable registered", "[observer][multi]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);
    observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);

    CHECK(observer.getObservableCount() == 1);
    observable.removeObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);
    CHECK(observer.getObservableCount() == 0);
}

// ---------------------------------------------------------------------------
// Classes that are Observer and Observable at the same time
// ---------------------------------------------------------------------------

class Relay final : public Observer, public Observable {
public:
    void onNotify(const UserEvent event) override {
        received.push_back(event.type);
        if (onReceive) onReceive(event);
    }

    void publish(const UserEvent& event) { notify(event); }
    size_t getObservableCount() const { return observable_.size(); }
    size_t getObserverCount() const {
        size_t count = 0;
        for (const auto& [type, list] : observers_) count += list.size();
        return count;
    }
    Observable* asObservable() { return this; }

    std::vector<EventType> received;
    std::function<void(const UserEvent&)> onReceive;
};

TEST_CASE("A relay forwards events through a chain of observers", "[observer][relay]") {
    TestObservable source;
    Relay relay;
    RecordingObserver sink;

    source.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &relay);
    relay.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &sink);
    relay.onReceive = [&](const UserEvent&) {
        relay.publish({EventType::BEHAVIOR_PRESET_APPLY, 7});
    };

    source.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    REQUIRE(relay.received.size() == 1);
    REQUIRE(sink.allReceivedEvents.size() == 1);
    CHECK(std::get<int>(sink.allReceivedEvents[0].payload) == 7);
}

TEST_CASE("Destroying a relay detaches it on both sides", "[observer][relay]") {
    TestObservable source;
    RecordingObserver sink;

    {
        Relay relay;
        source.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &relay);
        relay.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &sink);
        REQUIRE(sink.getObservableCount() == 1);
    }

    CHECK(source.getObserverCount() == 0);
    CHECK(sink.getObservableCount() == 0);
    source.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    CHECK(sink.allReceivedEvents.empty());
}

TEST_CASE("Destroying the upstream observable of a relay leaves the relay usable", "[observer][relay]") {
    Relay relay;
    RecordingObserver sink;
    relay.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &sink);

    {
        TestObservable source;
        source.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &relay);
        REQUIRE(relay.getObservableCount() == 1);
    }

    CHECK(relay.getObservableCount() == 0);
    relay.publish({EventType::BEHAVIOR_PRESET_APPLY, 0});
    CHECK(sink.allReceivedEvents.size() == 1);
}

TEST_CASE("An object may observe itself", "[observer][relay]") {
    Relay relay;
    relay.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &relay);

    relay.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    CHECK(relay.received.size() == 1);
    // The self reference is cleaned up in the destructor without crashing.
}

TEST_CASE("Two relays observing each other can be destroyed in either order", "[observer][relay]") {
    SECTION("first created is destroyed first") {
        auto a = std::make_unique<Relay>();
        auto b = std::make_unique<Relay>();
        a->addObserver(EventType::BEHAVIOR_PRESET_CREATE, b.get());
        b->addObserver(EventType::BEHAVIOR_PRESET_APPLY, a.get());

        a.reset();
        CHECK(b->getObservableCount() == 0);
        CHECK(b->getObserverCount() == 0);
        b->publish({EventType::BEHAVIOR_PRESET_APPLY, 0});
    }
    SECTION("second created is destroyed first") {
        auto a = std::make_unique<Relay>();
        auto b = std::make_unique<Relay>();
        a->addObserver(EventType::BEHAVIOR_PRESET_CREATE, b.get());
        b->addObserver(EventType::BEHAVIOR_PRESET_APPLY, a.get());

        b.reset();
        CHECK(a->getObservableCount() == 0);
        CHECK(a->getObserverCount() == 0);
        a->publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    }
}

TEST_CASE("Event ping-pong between relays terminates when the chain ends", "[observer][relay]") {
    Relay a;
    Relay b;
    a.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &b);
    b.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &a);

    // a -> b (CREATE), b answers with APPLY -> a, a does not answer: no endless loop.
    b.onReceive = [&](const UserEvent&) { b.publish({EventType::BEHAVIOR_PRESET_APPLY, 0}); };

    a.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(b.received.size() == 1);
    CHECK(a.received.size() == 1);
}

// ---------------------------------------------------------------------------
// Re-entrancy: changing subscriptions while a notification is running
// ---------------------------------------------------------------------------

class SelfRemovingObserver final : public Observer {
public:
    void onNotify(const UserEvent event) override {
        ++calls;
        if (observable) observable->removeObserverAll(this);
    }
    Observable* observable = nullptr;
    int calls = 0;
};

TEST_CASE("An observer may unsubscribe itself inside its callback", "[observer][reentrancy]") {
    TestObservable observable;
    SelfRemovingObserver remover;
    RecordingObserver other;
    remover.observable = &observable;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &remover);
    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &other);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(remover.calls == 1);
    CHECK(other.allReceivedEvents.size() == 2);
}

TEST_CASE("An observer removed by an earlier callback is not notified afterwards", "[observer][reentrancy]") {
    TestObservable observable;
    RecordingObserver victim;
    Relay killer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &killer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &victim);
    killer.onReceive = [&](const UserEvent&) {
        observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, &victim);
    };

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(victim.allReceivedEvents.empty());
}

TEST_CASE("Observers added during a notification do not break iteration", "[observer][reentrancy]") {
    TestObservable observable;
    RecordingObserver late;
    Relay adder;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &adder);
    adder.onReceive = [&](const UserEvent&) {
        observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &late);
    };

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    CHECK(late.allReceivedEvents.empty());   // takes effect from the next event

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    CHECK(late.allReceivedEvents.size() == 1);
}

TEST_CASE("Publishing a different event from inside a callback on the same observable works", "[observer][reentrancy]") {
    Relay relay;
    RecordingObserver sink;

    relay.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &relay);
    relay.addObserver(EventType::COLOR_PRESET_APPLY, &sink);   // new map keys while iterating
    relay.addObserver(EventType::IMAGE_PRESET_APPLY, &sink);
    relay.onReceive = [&](const UserEvent&) {
        relay.publish({EventType::COLOR_PRESET_APPLY, 0});
        relay.publish({EventType::IMAGE_PRESET_APPLY, 0});
        relay.publish({EventType::TEXT_PRESET_APPLY, 0});
    };

    relay.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(sink.allReceivedEvents.size() == 2);
}

TEST_CASE("A relay that unsubscribes itself while observing itself is safe", "[observer][reentrancy]") {
    Relay relay;
    relay.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &relay);
    relay.onReceive = [&](const UserEvent&) {
        relay.removeObserverAll(&relay);
    };

    relay.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});
    relay.publish({EventType::BEHAVIOR_PRESET_CREATE, 0});

    CHECK(relay.received.size() == 1);
    CHECK(relay.getObservableCount() == 0);
}

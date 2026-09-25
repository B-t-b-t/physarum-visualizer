#include <catch2/catch_test_macros.hpp>

#include <string>
#include <variant>
#include <vector>

#include "utility/event.h"
#include "utility/observable.h"
#include "utility/observer.h"

class RecordingObserver final : public Observer {
public:
    void onNotify(const UserEvent event) override {
        allReceivedEvents.push_back(event);
    }

    Observable* getObservable() const {
        return observable_;
    }

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
    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, std::string{"preset.psf"}, 2.0f});

    REQUIRE(observer.allReceivedEvents.size() == 1);
    CHECK(observer.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_CREATE);
    CHECK(std::get<std::string>(observer.allReceivedEvents[0].data_1) == "preset.psf");
    CHECK(std::get<float>(observer.allReceivedEvents[0].data_2) == 2.0f);
}

TEST_CASE("Observable only notifies observers subscribed to the matching event", "[observer]") {
    TestObservable observable;
    RecordingObserver saveObserver;
    RecordingObserver loadObserver;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &saveObserver);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &loadObserver);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 1, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 2, 0});

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

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0, 0});

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

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0, 0});

    REQUIRE(observer.allReceivedEvents.size() == 1);
    CHECK(observer.allReceivedEvents[0].type == EventType::BEHAVIOR_PRESET_APPLY);
}

TEST_CASE("Observable removes an observer from all event subscriptions", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.addObserver(EventType::BEHAVIOR_PRESET_APPLY, &observer);
    observable.removeObserverAll(&observer);

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0, 0});
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0, 0});

    CHECK(observer.allReceivedEvents.empty());
}

TEST_CASE("Observable safely handles missing subscriptions and null observers", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, nullptr);
    observable.removeObserver(EventType::BEHAVIOR_PRESET_CREATE, nullptr);
    observable.removeObserverAll(nullptr);

    observable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    observable.publish({EventType::BEHAVIOR_PRESET_APPLY, 0, 0});

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

    observable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0, 0});

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
        firstObservable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0, 0});
        REQUIRE(observer.getObservable() == &firstObservable);
    }

    //test sucessfull reset after observable destruction
    REQUIRE(observer.getObservable() == nullptr);

    TestObservable secondObservable;
    secondObservable.addObserver(EventType::BEHAVIOR_PRESET_CREATE, &observer);
    secondObservable.publish({EventType::BEHAVIOR_PRESET_CREATE, 0, 0});
    REQUIRE(observer.getObservable() == &secondObservable);

    REQUIRE(observer.allReceivedEvents.size() == 2);
}

TEST_CASE("Observable preserves complex event payloads", "[observer]") {
    TestObservable observable;
    RecordingObserver observer;

    const TrailMaskData mask{
        "night-mask",
        true,
        false,
        1,
        2,
        3,
        4,
        5,
        6
    };

    observable.addObserver(EventType::EDIT_TRAIL_MASK_TIME_SLOT, &observer);
    observable.publish({EventType::EDIT_TRAIL_MASK_TIME_SLOT, mask, 7});

    REQUIRE(observer.allReceivedEvents.size() == 1);

    const auto& receivedMask = std::get<TrailMaskData>(observer.allReceivedEvents[0].data_1);
    CHECK(receivedMask.newName == "night-mask");
    CHECK(receivedMask.isText);
    CHECK_FALSE(receivedMask.hasTimeSlot);
    CHECK(receivedMask.dayBegin == 1);
    CHECK(receivedMask.minuteEnd == 6);
    CHECK(std::get<int>(observer.allReceivedEvents[0].data_2) == 7);
}
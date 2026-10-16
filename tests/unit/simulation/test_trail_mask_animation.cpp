#include <catch2/catch_test_macros.hpp>

#include "simulation/trail_mask_animation.h"

TEST_CASE("Test hasReachedDestination", "[trail_mask_animation]") {

    SECTION("Translation") {
        phys::Vec2<double> velocity{0.0f, 0.0f};
        phys::Vec2<float> start{0.1f, 0.0f};
        phys::Vec2<float> end{1.0f, 0.0f};

        TrailMaskAnimation animation{start, end, 10.0f};
        phys::Vec2<float> scale{1.0f, 1.0f};

        // start position
        phys::Vec2<float> current_position = start;
        REQUIRE_FALSE(animation.hasReachedDest(current_position, scale));
        //somewhere in between start and end
        current_position = phys::Vec2<float>{0.05f, 0.0f};
        REQUIRE_FALSE(animation.hasReachedDest(current_position, scale));
        //end position
        current_position = end;
        REQUIRE(animation.hasReachedDest(current_position, scale));
        // slightly past the end position
        current_position = phys::Vec2<float>{1.004f, 0.0f};
        REQUIRE_FALSE(animation.hasReachedDest(current_position, scale));
        // false y positions
        current_position = phys::Vec2<float>{0.0f, 1.0f};
        REQUIRE_FALSE(animation.hasReachedDest(current_position, scale));
        current_position = phys::Vec2<float>{1.0f, 1.0f};
        REQUIRE_FALSE(animation.hasReachedDest(current_position, scale));
        current_position = phys::Vec2<float>{1.0f, 1.001f};
        REQUIRE_FALSE(animation.hasReachedDest(current_position, scale));
    }
    
}
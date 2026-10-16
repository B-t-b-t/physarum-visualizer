#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "catch2/matchers/catch_matchers.hpp"

#include "utility/vector_math.h"

using namespace phys;
using namespace Catch::Matchers;

TEST_CASE("Vector Math Operations", "[vector_math]") {
    
    SECTION("Float") {

        Vec2<float> a{1.2, 2.0};
        Vec2<float> b{3.0, 4.0};
        Vec2<float> c{5.0, 6.0};

        SECTION("Addition") {

        Vec2<float> result = a + b;
        REQUIRE_THAT(result.x, WithinRel(4.2f));
        REQUIRE_THAT(result.y, WithinRel(6.0f));
        
        result = a + b + c;
        REQUIRE_THAT(result.x, WithinRel(9.2f));
        REQUIRE_THAT(result.y, WithinRel(12.0f));

        result += b;
        REQUIRE_THAT(result.x, WithinRel(12.2f));
        REQUIRE_THAT(result.y, WithinRel(16.0f));
        }

        SECTION("Subtraction") {

        Vec2<float> result = a - b;
        REQUIRE_THAT(result.x, WithinRel(-1.8f));
        REQUIRE_THAT(result.y, WithinRel(-2.0f));

        result = a - b - c;
        REQUIRE_THAT(result.x, WithinRel(-6.8f));
        REQUIRE_THAT(result.y, WithinRel(-8.0f));

        result -= b;
        REQUIRE_THAT(result.x, WithinRel(-9.8f));
        REQUIRE_THAT(result.y, WithinRel(-12.0f));
        }
    }
}

TEST_CASE("Vector Comparisons", "[vector_math]") {
    
    SECTION("Integer") {

        Vec2<int> a{1, 2};
        Vec2<int> b{1, 3};
        Vec2<int> a_same{1, 2};
        Vec2<int> a_mirrored{1, -2};
        Vec2<int> negative_a{-1, -2};

        SECTION("Equality") {
            REQUIRE_FALSE(a == b);
            REQUIRE(a == a_same);
            REQUIRE_FALSE(a == a_mirrored);
            REQUIRE_FALSE(a == negative_a);
        }

        SECTION("Inequality") {
            REQUIRE(a != b);
            REQUIRE_FALSE(a != a_same);
            REQUIRE(a != a_mirrored);
            REQUIRE(a != negative_a);
        }
    }

    SECTION("Float") {

        Vec2<float> a{1.2, 2.0};
        Vec2<float> b{1.2, 2.001};
        Vec2<float> a_same{1.2, 2.0};
        Vec2<float> a_mirrored{1.2, -2.0};
        Vec2<float> negative_a{-1.2f, -2.0f};

        SECTION("Equality") {
            REQUIRE_FALSE(a == b);
            REQUIRE(a == a_same);
            REQUIRE_FALSE(a == a_mirrored);
            REQUIRE_FALSE(a == negative_a);
        }

        SECTION("Inequality") {
            REQUIRE(a != b);
            REQUIRE_FALSE(a != a_same);
            REQUIRE(a != a_mirrored);
            REQUIRE(a != negative_a);
        }
    }

    SECTION("Double") {

        Vec2<double> a{1.2, 2.0};
        Vec2<double> b{1.2, 2.001};
        Vec2<double> a_same{1.2, 2.0};
        Vec2<double> a_mirrored{1.2, -2.0};
        Vec2<double> negative_a{-1.2, -2.0};

        SECTION("Equality") {
            REQUIRE_FALSE(a == b);
            REQUIRE(a == a_same);
            REQUIRE_FALSE(a == a_mirrored);
            REQUIRE_FALSE(a == negative_a);
        }

        SECTION("Inequality") {
            REQUIRE(a != b);
            REQUIRE_FALSE(a != a_same);
            REQUIRE(a != a_mirrored);
            REQUIRE(a != negative_a);
        }
    }
}

TEST_CASE("Vector Length, Distance and Normalization", "[vector_math]") {

    SECTION("Length") {
        Vec2<float> a{3.0f, 4.0f};
        Vec2<float> a_negative{3.0f, -4.0f};
        Vec3<float> b{1.0f, 2.0f, 2.0f};
        Vec4<float> c{2.0f, 2.0f, 2.0f, 2.0f};
        Vec2<float> null_vec{0.0f, 0.0f};

        REQUIRE_THAT(VecMath<float>::length(a), WithinRel(5.0f));
        REQUIRE_THAT(VecMath<float>::length(a_negative), WithinRel(5.0f));
        REQUIRE_THAT(VecMath<float>::length(b), WithinRel(3.0f));
        REQUIRE_THAT(VecMath<float>::length(c), WithinRel(4.0f));
        REQUIRE_THAT(VecMath<float>::length(null_vec), WithinRel(0.0f));
    }

    SECTION("Distance") {
        Vec2<float> a{1.0f, 2.0f};
        Vec2<float> a_negative{-1.0f, -2.0f};
        Vec2<float> b{4.0f, 6.0f};
        Vec3<float> c{4.0f, 6.0f, 3.0f};
        Vec3<float> d{1.0f, 2.0f, 3.0f};
        Vec4<float> e{1.0f, 2.0f, 3.0f, 4.0f};
        Vec4<float> f{4.0f, 6.0f, 3.0f, 8.0f};
        Vec4<float> null_vec4{0.0f, 0.0f, 0.0f, 0.0f};

        REQUIRE_THAT(VecMath<float>::distance(a, b), WithinRel(5.0f));
        REQUIRE_THAT(VecMath<float>::distance(a, a_negative), WithinRel(2 * VecMath<float>::length(a)));
        REQUIRE_THAT(VecMath<float>::distance(c, d), WithinRel(5.0f));
        REQUIRE_THAT(VecMath<float>::distance(e, f), WithinRel(6.403124237f));
        REQUIRE_THAT(VecMath<float>::distance(e, null_vec4), WithinRel(VecMath<float>::length(e)));
    }
    
    SECTION("Normalization") {
        Vec2<float> a{3.0f, 4.0f};
        Vec2<float> a_negative{-3.0f, -4.0f};
        Vec2<float> null_vec{0.0f, 0.0f};
        Vec3<float> b{1.0f, 2.0f, 2.0f};
        Vec4<float> c{2.0f, 2.0f, 2.0f, 2.0f};

        auto a_normalized = VecMath<float>::normalize(a);
        auto a_negative_normalized = VecMath<float>::normalize(a_negative);
        auto null_vec_normalized = VecMath<float>::normalize(null_vec);
        auto b_normalized = VecMath<float>::normalize(b);
        auto c_normalized = VecMath<float>::normalize(c);

        REQUIRE_THAT(VecMath<float>::length(a_normalized), WithinRel(1.0f));
        REQUIRE_THAT(VecMath<float>::length(a_negative_normalized), WithinRel(1.0f));
        REQUIRE_THAT(VecMath<float>::length(null_vec_normalized), WithinRel(0.0f));
        REQUIRE_THAT(VecMath<float>::length(b_normalized), WithinRel(1.0f));
        REQUIRE_THAT(VecMath<float>::length(c_normalized), WithinRel(1.0f));
    }
}
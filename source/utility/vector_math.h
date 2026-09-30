#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

namespace phys {

    template<typename T>
    struct Vec2 {
        T x{0.0f};
        T y{0.0f};

        Vec2() = default;
        Vec2(T xVal, T yVal) : x(xVal), y(yVal) {}
    };

    template<typename T>
    struct Vec3 {
        T x{0.0f};
        T y{0.0f};
        T z{0.0f};

        Vec3() = default;
        Vec3(T xVal, T yVal, T zVal) : x(xVal), y(yVal), z(zVal) {}
    };

    template<typename T>
    struct Vec4 {
        T x{0.0f};
        T y{0.0f};
        T z{0.0f};
        T w{0.0f};

        Vec4() = default;
        Vec4(T xVal, T yVal, T zVal, T wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}
    };
}

#endif // VECTOR_MATH_H
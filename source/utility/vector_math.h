#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <cmath>

namespace phys {

    template<typename T>
    struct Vec2 {
        T x{0.0f};
        T y{0.0f};

        Vec2() = default;
        Vec2(T xVal, T yVal) : x(xVal), y(yVal) {}

        Vec2<T> operator+ (const Vec2<T>& other) const {
            return Vec2<T>(x + other.x, y + other.y);
        }

        Vec2<T> operator+= (const Vec2<T>& other) {
            x += other.x;
            y += other.y;
            return *this;
        }
        
        Vec2<T> operator- (const Vec2<T>& other) const {
            return Vec2<T>(x - other.x, y - other.y);
        }
        
        Vec2<T> operator-=(const Vec2<T>& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        auto operator<=>(const Vec2<T>&) const = default;
    };


    template<typename T>
    struct Vec3 {
        T x{0.0f};
        T y{0.0f};
        T z{0.0f};

        Vec3() = default;
        Vec3(T xVal, T yVal, T zVal) : x(xVal), y(yVal), z(zVal) {}
        
        Vec3<T> operator+ (const Vec3<T>& other) const {
            return Vec3<T>(x + other.x, y + other.y, z + other.z);
        }
        
        Vec3<T> operator+= (const Vec3<T>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }
        
        Vec3<T> operator- (const Vec3<T>& other) const {
            return Vec3<T>(x - other.x, y - other.y, z - other.z);
        }
        
        Vec3<T> operator-=(const Vec3<T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        auto operator<=>(const Vec3<T>&) const = default;
    };

    template<typename T>
    struct Vec4 {
        T x{0.0f};
        T y{0.0f};
        T z{0.0f};
        T w{0.0f};

        Vec4() = default;
        Vec4(T xVal, T yVal, T zVal, T wVal) : x(xVal), y(yVal), z(zVal), w(wVal) {}
        
        
        Vec4<T> operator+ (const Vec4<T>& other) const {
            return Vec4<T>(x + other.x, y + other.y, z + other.z, w + other.w);
        }
        
        Vec4<T> operator+= (const Vec4<T>& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }
        
        Vec4<T> operator- (const Vec4<T>& other) const {
            return Vec4<T>(x - other.x, y - other.y, z - other.z, w - other.w);
        }
        
        Vec4<T> operator-=(const Vec4<T>& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        auto operator<=>(const Vec4<T>&) const = default;
    };

    template<typename T>
    struct VecMath {
        static Vec2<T> abs(const Vec2<T>& vec) {
            return Vec2<T>(std::abs(vec.x), std::abs(vec.y));
        }

        static Vec3<T> abs(const Vec3<T>& vec) {
            return Vec3<T>(std::abs(vec.x), std::abs(vec.y), std::abs(vec.z));
        }

        static Vec4<T> abs(const Vec4<T>& vec) {
            return Vec4<T>(std::abs(vec.x), std::abs(vec.y), std::abs(vec.z), std::abs(vec.w));
        }
    };
}

#endif // VECTOR_MATH_H
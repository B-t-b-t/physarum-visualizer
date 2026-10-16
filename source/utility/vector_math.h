#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <cmath>

namespace phys {

    //example from cppreference: https://en.cppreference.com/cpp/types/numeric_limits/epsilon
    template <class T>
    requires std::floating_point<T>
    std::enable_if_t<not std::numeric_limits<T>::is_integer, bool>
    equal(T x, T y, std::size_t n) {
        // Since `epsilon()` is the gap size (ULP, unit in the last place)
        // of floating-point numbers in interval [1, 2), we can scale it to
        // the gap size in interval [2^e, 2^{e+1}), where `e` is the exponent
        // of `x` and `y`.

        // If `x` and `y` have different gap sizes (which means they have
        // different exponents), we take the smaller one. Taking the bigger
        // one is also reasonable, I guess.
        const T m = std::min(std::fabs(x), std::fabs(y));

        // Subnormal numbers have fixed exponent, which is `min_exponent - 1`.
        const int exp = m < std::numeric_limits<T>::min() ? std::numeric_limits<T>::min_exponent - 1 : std::ilogb(m);

        // We consider `x` and `y` equal if the difference between them is
        // within `n` ULPs.
        return std::fabs(x - y) <= n * std::ldexp(std::numeric_limits<T>::epsilon(), exp);
    }

    template<typename T>
    requires std::integral<T> || std::floating_point<T>
    struct Vec2 {
        T x{0.0f};
        T y{0.0f};

        Vec2() = default;
        Vec2(T xVal, T yVal) : x(xVal), y(yVal) {}

        Vec2<T> operator+ (const Vec2<T>& rhs) const {
            return Vec2<T>(x + rhs.x, y + rhs.y);
        }

        Vec2<T> operator+= (const Vec2<T>& rhs) {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
        
        Vec2<T> operator- (const Vec2<T>& rhs) const {
            return Vec2<T>(x - rhs.x, y - rhs.y);
        }
        
        Vec2<T> operator-=(const Vec2<T>& rhs) {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        template<typename U = T>
        requires std::floating_point<U>
        auto operator==(const Vec2<U>& rhs) const {
            return phys::equal(x, rhs.x, 1) && phys::equal(y, rhs.y, 1);
        }

        template<typename U = T>
        requires std::floating_point<U>
        auto operator!=(const Vec2<U>& rhs) const {
            return !phys::equal(x, rhs.x, 1) || !phys::equal(y, rhs.y, 1);
        }

        template<typename U = T>
        requires std::integral<U>
        auto operator==(const Vec2<U>& rhs) const {
            return x == rhs.x && y == rhs.y;
        }

        template<typename U = T>
        requires std::integral<U>
        auto operator!=(const Vec2<U>& rhs) const {
            return x != rhs.x || y != rhs.y;
        }
    };


    template<typename T>
    requires std::integral<T> || std::floating_point<T>
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

        template<typename U = T>
        requires std::floating_point<U>
        auto operator==(const Vec3<U>& rhs) const {
            return phys::equal(x, rhs.x, 1) && phys::equal(y, rhs.y, 1) && phys::equal(z, rhs.z, 1);
        }

        template<typename U = T>
        requires std::floating_point<U>
        auto operator!=(const Vec3<U>& rhs) const {
            return !phys::equal(x, rhs.x, 1) || !phys::equal(y, rhs.y, 1) || !phys::equal(z, rhs.z, 1);
        }
        
        template<typename U = T>
        requires std::integral<U>
        auto operator==(const Vec3<U>& rhs) const {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }

        template<typename U = T>
        requires std::integral<U>
        auto operator!=(const Vec3<U>& rhs) const {
            return x != rhs.x || y != rhs.y || z != rhs.z;
        }
    };

    template<typename T>
    requires std::integral<T> || std::floating_point<T>
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

        template<typename U = T>
        requires std::floating_point<U>
        auto operator==(const Vec4<U>& rhs) const {
            return phys::equal(x, rhs.x, 1) && phys::equal(y, rhs.y, 1) && phys::equal(z, rhs.z, 1) && phys::equal(w, rhs.w, 1);
        }

        template<typename U = T>
        requires std::floating_point<U>
        auto operator!=(const Vec4<U>& rhs) const {
            return !phys::equal(x, rhs.x, 1) || !phys::equal(y, rhs.y, 1) || !phys::equal(z, rhs.z, 1) || !phys::equal(w, rhs.w, 1);
        }
        
        template<typename U = T>
        requires std::integral<U>
        auto operator==(const Vec4<U>& rhs) const {
            return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
        }

        template<typename U = T>
        requires std::integral<U>
        auto operator!=(const Vec4<U>& rhs) const {
            return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
        }
    };

    template<typename T>
    requires std::integral<T> || std::floating_point<T>
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

        static auto length(const Vec2<T>& vec) {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y);
        }

        static auto length(const Vec3<T>& vec) {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
        }

        static auto length(const Vec4<T>& vec) {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
        }

        static auto distance(const Vec2<T>& a, const Vec2<T>& b) {
            return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
        }
        
        static auto distance(const Vec3<T>& a, const Vec3<T>& b) {
            return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
        }

        static auto distance(const Vec4<T>& a, const Vec4<T>& b) {
            return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z) + (a.w - b.w) * (a.w - b.w));
        }

        static auto normalize(const Vec2<T>& vec) {
            auto len = length(vec);
            return len > 0 ? Vec2<T>(vec.x / len, vec.y / len) : Vec2<T>(0, 0);
        }
        
        static auto normalize(const Vec3<T>& vec) {
            auto len = length(vec);
            return len > 0 ? Vec3<T>(vec.x / len, vec.y / len, vec.z / len) : Vec3<T>(0, 0, 0);
        }

        static auto normalize(const Vec4<T>& vec) {
            auto len = length(vec);
            return len > 0 ? Vec4<T>(vec.x / len, vec.y / len, vec.z / len, vec.w / len) : Vec4<T>(0, 0, 0, 0);
        }
    };
}

#endif // VECTOR_MATH_H
#pragma once
#include <cstdint>
#include <cstddef>

enum class Error: uint8_t {
    Ok = 0,            // No error
    Unknown,           // Used for unspecified errors
    InvalidParameters, // Used when function parameters are invalid
    NotFound,          // Used when a requested item is not found
    NoMemory,          // Used when memory allocation fails or memory is insufficient
};

template<typename T>
inline T map(T x, T in_min, T in_max, T out_min, T out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template<typename T>
class Vec3
{
public:
    T x;
    T y;
    T z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    Vec3<T> operator+(const Vec3<T>& other) const {
        return Vec3<T>(x + other.x, y + other.y, z + other.z);
    }
    Vec3<T> operator-(const Vec3<T>& other) const {
        return Vec3<T>(x - other.x, y - other.y, z - other.z);
    }
    Vec3<T> operator*(T scalar) const {
        return Vec3<T>(x * scalar, y * scalar, z * scalar);
    }
    Vec3<T> operator/(T scalar) const {
        return Vec3<T>(x / scalar, y / scalar, z / scalar);
    }
    Vec3<T>& operator+=(const Vec3<T>& other) {
        x += other.x; y += other.y; z += other.z; return *this;
    }
    Vec3<T>& operator-=(const Vec3<T>& other) {
        x -= other.x; y -= other.y; z -= other.z; return *this;
    }
    Vec3<T>& operator*=(T scalar) {
        x *= scalar; y *= scalar; z *= scalar; return *this;
    }
    Vec3<T>& operator/=(T scalar) {
        x /= scalar; y /= scalar; z /= scalar; return *this;
    }
};
using Vec3f = Vec3<float>;
using Vec3i = Vec3<int>;

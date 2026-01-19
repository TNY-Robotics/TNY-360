#pragma once
#include <cmath>

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float DEG_TO_RAD_FACTOR = PI / 180.0f;
constexpr float RAD_TO_DEG_FACTOR = 180.0f / PI;

#define DEG_TO_RAD(deg) ((deg) * DEG_TO_RAD_FACTOR)
#define RAD_TO_DEG(rad) ((rad) * RAD_TO_DEG_FACTOR)

template<typename T>
class Vec3
{
public:
    T x;
    T y;
    T z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
    
    T dot(const Vec3<T>& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3<T> cross(const Vec3<T>& other) const {
        return Vec3<T>(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    T lengthSq() const { return x * x + y * y + z * z; }
    
    T length() const { return std::sqrt(lengthSq()); }

    Vec3<T> normalized() const {
        T len = length();
        if (len > 0) return *this / len;
        return *this;
    }

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


template<typename T>
class Vec2
{
public:
    T x;
    T y;

    Vec2() : x(0), y(0) {}
    Vec2(T x, T y) : x(x), y(y) {}
    Vec2<T> operator+(const Vec2<T>& other) const {
        return Vec2<T>(x + other.x, y + other.y);
    }
    Vec2<T> operator-(const Vec2<T>& other) const {
        return Vec2<T>(x - other.x, y - other.y);
    }
    Vec2<T> operator*(T scalar) const {
        return Vec2<T>(x * scalar, y * scalar);
    }
    Vec2<T> operator/(T scalar) const {
        return Vec2<T>(x / scalar, y / scalar);
    }
    Vec2<T>& operator+=(const Vec2<T>& other) {
        x += other.x; y += other.y; return *this;
    }
    Vec2<T>& operator-=(const Vec2<T>& other) {
        x -= other.x; y -= other.y; return *this;
    }
    Vec2<T>& operator*=(T scalar) {
        x *= scalar; y *= scalar; return *this;
    }
    Vec2<T>& operator/=(T scalar) {
        x /= scalar; y /= scalar; return *this;
    }
};
using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;


template<typename T>
class Quat
{
public:
    static Quat<T> FromEulerAngles(Vec3<T> angles) {
        float cy = cos(angles.z * 0.5f);
        float sy = sin(angles.z * 0.5f);
        float cp = cos(angles.y * 0.5f);
        float sp = sin(angles.y * 0.5f);
        float cr = cos(angles.x * 0.5f);
        float sr = sin(angles.x * 0.5f);

        Quat<T> q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;

        return q;
    }

    T x;
    T y;
    T z;
    T w;

    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    Vec3<T> rotate(const Vec3<T>& v) const {
        Vec3<T> q_vec(x, y, z);        
        Vec3<T> t = q_vec.cross(v) * (T)2;
        return v + (t * w) + q_vec.cross(t);
    }

    Quat<T> conjugate() const {
        return Quat<T>(-x, -y, -z, w);
    }

    Vec3<T> toEulerAngles() const {
        Vec3<T> angles;

        // Roll (x-axis rotation)
        T sinr_cosp = 2 * (w * x + y * z);
        T cosr_cosp = 1 - 2 * (x * x + y * y);
        angles.x = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        T sinp = 2 * (w * y - z * x);
        if (std::abs(sinp) >= 1)
            angles.y = std::copysign(PI / 2, sinp); // use 90 degrees if out of range
        else
            angles.y = std::asin(sinp);

        // Yaw (z-axis rotation)
        T siny_cosp = 2 * (w * z + x * y);
        T cosy_cosp = 1 - 2 * (y * y + z * z);
        angles.z = std::atan2(siny_cosp, cosy_cosp);

        return angles;
    }

    void normalize() {
        T len = std::sqrt(w*w + x*x + y*y + z*z);
        if (len > 0) {
            w /= len; x /= len; y /= len; z /= len;
        }
    }

    Quat<T> operator*(const Quat<T>& other) const {
        return Quat<T>(
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        );
    }

    Quat<T>& operator*=(const Quat<T>& other) {
        *this = *this * other;
        return *this;
    }
};
using Quatf = Quat<float>;


template<typename T>
class Transform
{
public:
    Vec3<T> position;
    Quat<T> rotation;

    Transform() : position(), rotation() {}
    Transform(const Vec3<T>& position, const Quat<T>& rotation)
        : position(position), rotation(rotation) {}

    Vec3<T> worldToLocal(const Vec3<T>& point) const {
        Vec3<T> translated = point - position;
        return rotation.conjugate().rotate(translated);
    }

    Vec3<T> localToWorld(const Vec3<T>& point) const {
        Vec3<T> rotated = rotation.rotate(point);
        return rotated + position;
    }
};
using Transformf = Transform<float>;
#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector2.h"
#include <cmath>

namespace Avero::Math
{
    using Avero::Core::float32;

    struct Vector3
    {
        float32 X = 0.0f;
        float32 Y = 0.0f;
        float32 Z = 0.0f;

        constexpr Vector3() = default;
        constexpr Vector3(float32 x, float32 y, float32 z) : X(x), Y(y), Z(z) {}
        explicit constexpr Vector3(float32 scalar) : X(scalar), Y(scalar), Z(scalar) {}
        constexpr Vector3(const Vector2& xy, float32 z) : X(xy.X), Y(xy.Y), Z(z) {}

        static constexpr Vector3 Zero() { return Vector3(0.0f, 0.0f, 0.0f); }
        static constexpr Vector3 One() { return Vector3(1.0f, 1.0f, 1.0f); }
        static constexpr Vector3 UnitX() { return Vector3(1.0f, 0.0f, 0.0f); }
        static constexpr Vector3 UnitY() { return Vector3(0.0f, 1.0f, 0.0f); }
        static constexpr Vector3 UnitZ() { return Vector3(0.0f, 0.0f, 1.0f); }
        static constexpr Vector3 Up() { return UnitY(); }
        static constexpr Vector3 Forward() { return UnitZ(); }
        static constexpr Vector3 Right() { return UnitX(); }

        constexpr Vector3 operator+(const Vector3& other) const
        {
            return Vector3(X + other.X, Y + other.Y, Z + other.Z);
        }

        constexpr Vector3 operator-(const Vector3& other) const
        {
            return Vector3(X - other.X, Y - other.Y, Z - other.Z);
        }

        constexpr Vector3 operator*(float32 scalar) const
        {
            return Vector3(X * scalar, Y * scalar, Z * scalar);
        }

        constexpr Vector3 operator/(float32 scalar) const
        {
            return Vector3(X / scalar, Y / scalar, Z / scalar);
        }

        Vector3& operator+=(const Vector3& other)
        {
            X += other.X;
            Y += other.Y;
            Z += other.Z;
            return *this;
        }

        Vector3& operator-=(const Vector3& other)
        {
            X -= other.X;
            Y -= other.Y;
            Z -= other.Z;
            return *this;
        }

        Vector3& operator*=(float32 scalar)
        {
            X *= scalar;
            Y *= scalar;
            Z *= scalar;
            return *this;
        }

        Vector3& operator/=(float32 scalar)
        {
            X /= scalar;
            Y /= scalar;
            Z /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vector3& other) const
        {
            return X == other.X && Y == other.Y && Z == other.Z;
        }

        constexpr bool operator!=(const Vector3& other) const
        {
            return !(*this == other);
        }

        float32 Length() const
        {
            return std::sqrt(X * X + Y * Y + Z * Z);
        }

        float32 LengthSquared() const
        {
            return X * X + Y * Y + Z * Z;
        }

        float32 Dot(const Vector3& other) const
        {
            return X * other.X + Y * other.Y + Z * other.Z;
        }

        constexpr Vector3 Cross(const Vector3& other) const
        {
            return Vector3(
                Y * other.Z - Z * other.Y,
                Z * other.X - X * other.Z,
                X * other.Y - Y * other.X
            );
        }

        Vector3 Normalized() const
        {
            float32 len = Length();
            if (len > Avero::Core::EPSILON)
            {
                return Vector3(X / len, Y / len, Z / len);
            }
            return Zero();
        }

        void Normalize()
        {
            *this = Normalized();
        }

        constexpr Vector3 Lerp(const Vector3& other, float32 t) const
        {
            return Vector3(
                X + (other.X - X) * t,
                Y + (other.Y - Y) * t,
                Z + (other.Z - Z) * t
            );
        }

        constexpr Vector2 XY() const
        {
            return Vector2(X, Y);
        }
    };

    constexpr Vector3 operator*(float32 scalar, const Vector3& vec)
    {
        return vec * scalar;
    }
}
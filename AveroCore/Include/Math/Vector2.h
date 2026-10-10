#pragma once

#include "Core/AveroTypes.h"
#include <cmath>

namespace Avero::Math
{
    using Avero::Core::float32;

    struct Vector2
    {
        float32 X = 0.0f;
        float32 Y = 0.0f;

        constexpr Vector2() = default;
        constexpr Vector2(float32 x, float32 y) : X(x), Y(y) {}
        explicit constexpr Vector2(float32 scalar) : X(scalar), Y(scalar) {}

        static constexpr Vector2 Zero() { return Vector2(0.0f, 0.0f); }
        static constexpr Vector2 One() { return Vector2(1.0f, 1.0f); }
        static constexpr Vector2 UnitX() { return Vector2(1.0f, 0.0f); }
        static constexpr Vector2 UnitY() { return Vector2(0.0f, 1.0f); }

        constexpr Vector2 operator+(const Vector2& other) const
        {
            return Vector2(X + other.X, Y + other.Y);
        }

        constexpr Vector2 operator-(const Vector2& other) const
        {
            return Vector2(X - other.X, Y - other.Y);
        }

        constexpr Vector2 operator*(float32 scalar) const
        {
            return Vector2(X * scalar, Y * scalar);
        }

        constexpr Vector2 operator/(float32 scalar) const
        {
            return Vector2(X / scalar, Y / scalar);
        }

        Vector2& operator+=(const Vector2& other)
        {
            X += other.X;
            Y += other.Y;
            return *this;
        }

        Vector2& operator-=(const Vector2& other)
        {
            X -= other.X;
            Y -= other.Y;
            return *this;
        }

        Vector2& operator*=(float32 scalar)
        {
            X *= scalar;
            Y *= scalar;
            return *this;
        }

        Vector2& operator/=(float32 scalar)
        {
            X /= scalar;
            Y /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vector2& other) const
        {
            return X == other.X && Y == other.Y;
        }

        constexpr bool operator!=(const Vector2& other) const
        {
            return !(*this == other);
        }

        float32 Length() const
        {
            return std::sqrt(X * X + Y * Y);
        }

        float32 LengthSquared() const
        {
            return X * X + Y * Y;
        }

        float32 Dot(const Vector2& other) const
        {
            return X * other.X + Y * other.Y;
        }

        Vector2 Normalized() const
        {
            float32 len = Length();
            if (len > Avero::Core::EPSILON)
            {
                return Vector2(X / len, Y / len);
            }
            return Zero();
        }

        void Normalize()
        {
            *this = Normalized();
        }

        constexpr Vector2 Lerp(const Vector2& other, float32 t) const
        {
            return Vector2(
                X + (other.X - X) * t,
                Y + (other.Y - Y) * t
            );
        }
    };

    constexpr Vector2 operator*(float32 scalar, const Vector2& vec)
    {
        return vec * scalar;
    }
}
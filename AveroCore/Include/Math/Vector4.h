#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector3.h"
#include <cmath>

namespace Avero::Math
{
    using Avero::Core::float32;

    struct Vector4
    {
        float32 X = 0.0f;
        float32 Y = 0.0f;
        float32 Z = 0.0f;
        float32 W = 1.0f;

        constexpr Vector4() = default;
        constexpr Vector4(float32 x, float32 y, float32 z, float32 w)
            : X(x), Y(y), Z(z), W(w) {
        }
        explicit constexpr Vector4(float32 scalar)
            : X(scalar), Y(scalar), Z(scalar), W(scalar) {
        }
        constexpr Vector4(const Vector3& xyz, float32 w)
            : X(xyz.X), Y(xyz.Y), Z(xyz.Z), W(w) {
        }
        constexpr Vector4(const Vector2& xy, float32 z, float32 w)
            : X(xy.X), Y(xy.Y), Z(z), W(w) {
        }

        static constexpr Vector4 Zero() { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Vector4 One() { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
        static constexpr Vector4 UnitW() { return Vector4(0.0f, 0.0f, 0.0f, 1.0f); }

        constexpr Vector4 operator+(const Vector4& other) const
        {
            return Vector4(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
        }

        constexpr Vector4 operator-(const Vector4& other) const
        {
            return Vector4(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
        }

        constexpr Vector4 operator*(float32 scalar) const
        {
            return Vector4(X * scalar, Y * scalar, Z * scalar, W * scalar);
        }

        constexpr Vector4 operator/(float32 scalar) const
        {
            return Vector4(X / scalar, Y / scalar, Z / scalar, W / scalar);
        }

        Vector4& operator+=(const Vector4& other)
        {
            X += other.X;
            Y += other.Y;
            Z += other.Z;
            W += other.W;
            return *this;
        }

        Vector4& operator-=(const Vector4& other)
        {
            X -= other.X;
            Y -= other.Y;
            Z -= other.Z;
            W -= other.W;
            return *this;
        }

        Vector4& operator*=(float32 scalar)
        {
            X *= scalar;
            Y *= scalar;
            Z *= scalar;
            W *= scalar;
            return *this;
        }

        Vector4& operator/=(float32 scalar)
        {
            X /= scalar;
            Y /= scalar;
            Z /= scalar;
            W /= scalar;
            return *this;
        }

        constexpr bool operator==(const Vector4& other) const
        {
            return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
        }

        constexpr bool operator!=(const Vector4& other) const
        {
            return !(*this == other);
        }

        float32 Length() const
        {
            return std::sqrt(X * X + Y * Y + Z * Z + W * W);
        }

        float32 LengthSquared() const
        {
            return X * X + Y * Y + Z * Z + W * W;
        }

        float32 Dot(const Vector4& other) const
        {
            return X * other.X + Y * other.Y + Z * other.Z + W * other.W;
        }

        Vector4 Normalized() const
        {
            float32 len = Length();
            if (len > Avero::Core::EPSILON)
            {
                return Vector4(X / len, Y / len, Z / len, W / len);
            }
            return Zero();
        }

        void Normalize()
        {
            *this = Normalized();
        }

        constexpr Vector4 Lerp(const Vector4& other, float32 t) const
        {
            return Vector4(
                X + (other.X - X) * t,
                Y + (other.Y - Y) * t,
                Z + (other.Z - Z) * t,
                W + (other.W - W) * t
            );
        }

        constexpr Vector3 XYZ() const
        {
            return Vector3(X, Y, Z);
        }

        constexpr Vector2 XY() const
        {
            return Vector2(X, Y);
        }
    };

    constexpr Vector4 operator*(float32 scalar, const Vector4& vec)
    {
        return vec * scalar;
    }
}
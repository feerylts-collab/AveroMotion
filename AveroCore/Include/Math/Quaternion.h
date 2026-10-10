#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include <cmath>
#include <algorithm>

namespace Avero::Math
{
    using Avero::Core::float32;

    struct Quaternion
    {
        float32 X = 0.0f;
        float32 Y = 0.0f;
        float32 Z = 0.0f;
        float32 W = 1.0f;

        constexpr Quaternion() = default;
        constexpr Quaternion(float32 x, float32 y, float32 z, float32 w)
            : X(x), Y(y), Z(z), W(w) {
        }

        static constexpr Quaternion Identity()
        {
            return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
        }

        static Quaternion FromEuler(const Vector3& euler)
        {
            float32 cx = std::cos(euler.X * 0.5f);
            float32 sx = std::sin(euler.X * 0.5f);
            float32 cy = std::cos(euler.Y * 0.5f);
            float32 sy = std::sin(euler.Y * 0.5f);
            float32 cz = std::cos(euler.Z * 0.5f);
            float32 sz = std::sin(euler.Z * 0.5f);

            return Quaternion(
                sx * cy * cz - cx * sy * sz,
                cx * sy * cz + sx * cy * sz,
                cx * cy * sz - sx * sy * cz,
                cx * cy * cz + sx * sy * sz
            );
        }

        static Quaternion FromAxisAngle(const Vector3& axis, float32 angle)
        {
            Vector3 normalizedAxis = axis.Normalized();
            float32 halfAngle = angle * 0.5f;
            float32 s = std::sin(halfAngle);

            return Quaternion(
                normalizedAxis.X * s,
                normalizedAxis.Y * s,
                normalizedAxis.Z * s,
                std::cos(halfAngle)
            );
        }

        float32 Length() const
        {
            return std::sqrt(X * X + Y * Y + Z * Z + W * W);
        }

        Quaternion Normalized() const
        {
            float32 len = Length();
            if (len > Avero::Core::EPSILON)
            {
                return Quaternion(X / len, Y / len, Z / len, W / len);
            }
            return Identity();
        }

        void Normalize()
        {
            *this = Normalized();
        }

        Quaternion Conjugate() const
        {
            return Quaternion(-X, -Y, -Z, W);
        }

        Quaternion Inverse() const
        {
            float32 lenSq = X * X + Y * Y + Z * Z + W * W;
            if (lenSq > Avero::Core::EPSILON)
            {
                float32 inv = 1.0f / lenSq;
                return Quaternion(-X * inv, -Y * inv, -Z * inv, W * inv);
            }
            return Identity();
        }

        Quaternion operator*(const Quaternion& other) const
        {
            return Quaternion(
                W * other.X + X * other.W + Y * other.Z - Z * other.Y,
                W * other.Y - X * other.Z + Y * other.W + Z * other.X,
                W * other.Z + X * other.Y - Y * other.X + Z * other.W,
                W * other.W - X * other.X - Y * other.Y - Z * other.Z
            );
        }

        Vector3 operator*(const Vector3& vec) const
        {
            Vector3 qVec(X, Y, Z);
            Vector3 t = qVec.Cross(vec) * 2.0f;
            return vec + t * W + qVec.Cross(t);
        }

        float32 Dot(const Quaternion& other) const
        {
            return X * other.X + Y * other.Y + Z * other.Z + W * other.W;
        }

        static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float32 t)
        {
            float32 dot = a.Dot(b);

            // Clamp dot to [-1, 1] для защиты от floating-point ошибок
            dot = std::clamp(dot, -1.0f, 1.0f);

            Quaternion bAdj = b;
            if (dot < 0.0f)
            {
                bAdj = Quaternion(-b.X, -b.Y, -b.Z, -b.W);
                dot = -dot;
            }

            if (dot > 0.9995f)
            {
                return Quaternion(
                    a.X + t * (bAdj.X - a.X),
                    a.Y + t * (bAdj.Y - a.Y),
                    a.Z + t * (bAdj.Z - a.Z),
                    a.W + t * (bAdj.W - a.W)
                ).Normalized();
            }

            float32 theta0 = std::acos(dot);
            float32 theta = theta0 * t;
            float32 sinTheta = std::sin(theta);
            float32 sinTheta0 = std::sin(theta0);

            float32 s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
            float32 s1 = sinTheta / sinTheta0;

            return Quaternion(
                s0 * a.X + s1 * bAdj.X,
                s0 * a.Y + s1 * bAdj.Y,
                s0 * a.Z + s1 * bAdj.Z,
                s0 * a.W + s1 * bAdj.W
            );
        }

        Matrix4x4 ToMatrix() const
        {
            Matrix4x4 result;

            float32 xx = X * X;
            float32 yy = Y * Y;
            float32 zz = Z * Z;
            float32 xy = X * Y;
            float32 xz = X * Z;
            float32 yz = Y * Z;
            float32 wx = W * X;
            float32 wy = W * Y;
            float32 wz = W * Z;

            result.M[0][0] = 1.0f - 2.0f * (yy + zz);
            // Keep the matrix convention identical to Quaternion::operator*(Vector3).
            // Positive Z rotation therefore maps +X toward +Y.
            result.M[0][1] = 2.0f * (xy - wz);
            result.M[0][2] = 2.0f * (xz + wy);

            result.M[1][0] = 2.0f * (xy + wz);
            result.M[1][1] = 1.0f - 2.0f * (xx + zz);
            result.M[1][2] = 2.0f * (yz - wx);

            result.M[2][0] = 2.0f * (xz - wy);
            result.M[2][1] = 2.0f * (yz + wx);

            result.M[2][2] = 1.0f - 2.0f * (xx + yy);

            return result;
        }
    };
}
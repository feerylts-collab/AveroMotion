#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#include <cmath>
#include <cstring>
#include <algorithm>

namespace Avero::Math
{
    using Avero::Core::float32;

    /*
     * MATRIX CONTRACT:
     * - Handedness: Right-Handed (RH)
     * - Up Axis: +Y
     * - Forward Axis: -Z
     * - Layout: Column-Major (M[row][col] in C++, mathematically treated as columns)
     * - Depth Range: [0, 1] (Vulkan/Metal native)
     * - Multiplication Order: Projection * View * World * Local * Vector
     */
    struct Matrix4x4
    {
        float32 M[4][4];

        Matrix4x4()
        {
            std::memset(M, 0, sizeof(M));
            M[0][0] = 1.0f;
            M[1][1] = 1.0f;
            M[2][2] = 1.0f;
            M[3][3] = 1.0f;
        }

        static Matrix4x4 Identity() { return Matrix4x4(); }

        static Matrix4x4 Translation(const Vector3& pos)
        {
            Matrix4x4 result;
            result.M[0][3] = pos.X;
            result.M[1][3] = pos.Y;
            result.M[2][3] = pos.Z;
            return result;
        }

        static Matrix4x4 Scale(const Vector3& scale)
        {
            Matrix4x4 result;
            result.M[0][0] = scale.X;
            result.M[1][1] = scale.Y;
            result.M[2][2] = scale.Z;
            return result;
        }

        static Matrix4x4 RotationX(float32 angle)
        {
            Matrix4x4 result;
            float32 c = std::cos(angle);
            float32 s = std::sin(angle);
            result.M[1][1] = c;
            result.M[1][2] = -s;
            result.M[2][1] = s;
            result.M[2][2] = c;
            return result;
        }

        static Matrix4x4 RotationY(float32 angle)
        {
            Matrix4x4 result;
            float32 c = std::cos(angle);
            float32 s = std::sin(angle);
            result.M[0][0] = c;
            result.M[0][2] = s;
            result.M[2][0] = -s;
            result.M[2][2] = c;
            return result;
        }

        static Matrix4x4 RotationZ(float32 angle)
        {
            Matrix4x4 result;
            float32 c = std::cos(angle);
            float32 s = std::sin(angle);
            result.M[0][0] = c;
            result.M[0][1] = -s;
            result.M[1][0] = s;
            result.M[1][1] = c;
            return result;
        }

        // RH Perspective с диапазоном глубины [0, 1] (Vulkan/Metal)
        static Matrix4x4 Perspective(float32 fov, float32 aspect, float32 near, float32 far)
        {
            Matrix4x4 result;
            std::memset(result.M, 0, sizeof(result.M));

            float32 tanHalfFov = std::tan(fov / 2.0f);

            result.M[0][0] = 1.0f / (aspect * tanHalfFov);
            result.M[1][1] = 1.0f / tanHalfFov;

            // Исправлено: M[2][3] это Z bias, M[3][2] это проекция -Z в W
            result.M[2][2] = far / (near - far);
            result.M[2][3] = (far * near) / (near - far);
            result.M[3][2] = -1.0f;
            result.M[3][3] = 0.0f;

            return result;
        }

        // RH Orthographic с диапазоном глубины [0, 1]
        static Matrix4x4 Orthographic(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far)
        {
            Matrix4x4 result;
            std::memset(result.M, 0, sizeof(result.M));

            result.M[0][0] = 2.0f / (right - left);
            result.M[1][1] = 2.0f / (top - bottom);
            result.M[2][2] = 1.0f / (near - far);

            result.M[0][3] = -(right + left) / (right - left);
            result.M[1][3] = -(top + bottom) / (top - bottom);
            result.M[2][3] = near / (near - far);
            result.M[3][3] = 1.0f;

            return result;
        }

        Matrix4x4 operator*(const Matrix4x4& other) const
        {
            Matrix4x4 result;
            std::memset(result.M, 0, sizeof(result.M));

            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 4; row++)
                {
                    result.M[row][col] =
                        M[row][0] * other.M[0][col] +
                        M[row][1] * other.M[1][col] +
                        M[row][2] * other.M[2][col] +
                        M[row][3] * other.M[3][col];
                }
            }
            return result;
        }

        Vector4 operator*(const Vector4& vec) const
        {
            return Vector4(
                M[0][0] * vec.X + M[0][1] * vec.Y + M[0][2] * vec.Z + M[0][3] * vec.W,
                M[1][0] * vec.X + M[1][1] * vec.Y + M[1][2] * vec.Z + M[1][3] * vec.W,
                M[2][0] * vec.X + M[2][1] * vec.Y + M[2][2] * vec.Z + M[2][3] * vec.W,
                M[3][0] * vec.X + M[3][1] * vec.Y + M[3][2] * vec.Z + M[3][3] * vec.W
            );
        }

        Vector3 TransformPoint(const Vector3& point) const
        {
            Vector4 result = (*this) * Vector4(point.X, point.Y, point.Z, 1.0f);
            if (std::abs(result.W) > Avero::Core::EPSILON)
            {
                return Vector3(result.X / result.W, result.Y / result.W, result.Z / result.W);
            }
            return Vector3(result.X, result.Y, result.Z);
        }

        Vector3 TransformDirection(const Vector3& dir) const
        {
            Vector4 result = (*this) * Vector4(dir.X, dir.Y, dir.Z, 0.0f);
            return Vector3(result.X, result.Y, result.Z);
        }

        Matrix4x4 Transposed() const
        {
            Matrix4x4 result;
            for (int row = 0; row < 4; row++)
            {
                for (int col = 0; col < 4; col++)
                {
                    result.M[row][col] = M[col][row];
                }
            }
            return result;
        }

        Matrix4x4 Inverse() const
        {
            Matrix4x4 inv;
            inv.M[0][0] = M[1][1] * M[2][2] * M[3][3] - M[1][1] * M[2][3] * M[3][2] - M[2][1] * M[1][2] * M[3][3] + M[2][1] * M[1][3] * M[3][2] + M[3][1] * M[1][2] * M[2][3] - M[3][1] * M[1][3] * M[2][2];
            inv.M[1][0] = -M[1][0] * M[2][2] * M[3][3] + M[1][0] * M[2][3] * M[3][2] + M[2][0] * M[1][2] * M[3][3] - M[2][0] * M[1][3] * M[3][2] - M[3][0] * M[1][2] * M[2][3] + M[3][0] * M[1][3] * M[2][2];
            inv.M[2][0] = M[1][0] * M[2][1] * M[3][3] - M[1][0] * M[2][3] * M[3][1] - M[2][0] * M[1][1] * M[3][3] + M[2][0] * M[1][3] * M[3][1] + M[3][0] * M[1][1] * M[2][3] - M[3][0] * M[1][3] * M[2][1];
            inv.M[3][0] = -M[1][0] * M[2][1] * M[3][2] + M[1][0] * M[2][2] * M[3][1] + M[2][0] * M[1][1] * M[3][2] - M[2][0] * M[1][2] * M[3][1] - M[3][0] * M[1][1] * M[2][2] + M[3][0] * M[1][2] * M[2][1];
            inv.M[0][1] = -M[0][1] * M[2][2] * M[3][3] + M[0][1] * M[2][3] * M[3][2] + M[2][1] * M[0][2] * M[3][3] - M[2][1] * M[0][3] * M[3][2] - M[3][1] * M[0][2] * M[2][3] + M[3][1] * M[0][3] * M[2][2];
            inv.M[1][1] = M[0][0] * M[2][2] * M[3][3] - M[0][0] * M[2][3] * M[3][2] - M[2][0] * M[0][2] * M[3][3] + M[2][0] * M[0][3] * M[3][2] + M[3][0] * M[0][2] * M[2][3] - M[3][0] * M[0][3] * M[2][2];
            inv.M[2][1] = -M[0][0] * M[2][1] * M[3][3] + M[0][0] * M[2][3] * M[3][1] + M[2][0] * M[0][1] * M[3][3] - M[2][0] * M[0][3] * M[3][1] - M[3][0] * M[0][1] * M[2][3] + M[3][0] * M[0][3] * M[2][1];
            inv.M[3][1] = M[0][0] * M[2][1] * M[3][2] - M[0][0] * M[2][2] * M[3][1] - M[2][0] * M[0][1] * M[3][2] + M[2][0] * M[0][2] * M[3][1] + M[3][0] * M[0][1] * M[2][2] - M[3][0] * M[0][2] * M[2][1];
            inv.M[0][2] = M[0][1] * M[1][2] * M[3][3] - M[0][1] * M[1][3] * M[3][2] - M[1][1] * M[0][2] * M[3][3] + M[1][1] * M[0][3] * M[3][2] + M[3][1] * M[0][2] * M[1][3] - M[3][1] * M[0][3] * M[1][2];
            inv.M[1][2] = -M[0][0] * M[1][2] * M[3][3] + M[0][0] * M[1][3] * M[3][2] + M[1][0] * M[0][2] * M[3][3] - M[1][0] * M[0][3] * M[3][2] - M[3][0] * M[0][2] * M[1][3] + M[3][0] * M[0][3] * M[1][2];
            inv.M[2][2] = M[0][0] * M[1][1] * M[3][3] - M[0][0] * M[1][3] * M[3][1] - M[1][0] * M[0][1] * M[3][3] + M[1][0] * M[0][3] * M[3][1] + M[3][0] * M[0][1] * M[1][3] - M[3][0] * M[0][3] * M[1][1];
            inv.M[3][2] = -M[0][0] * M[1][1] * M[3][2] + M[0][0] * M[1][2] * M[3][1] + M[1][0] * M[0][1] * M[3][2] - M[1][0] * M[0][2] * M[3][1] - M[3][0] * M[0][1] * M[1][2] + M[3][0] * M[0][2] * M[1][1];
            inv.M[0][3] = -M[0][1] * M[1][2] * M[2][3] + M[0][1] * M[1][3] * M[2][2] + M[1][1] * M[0][2] * M[2][3] - M[1][1] * M[0][3] * M[2][2] - M[2][1] * M[0][2] * M[1][3] + M[2][1] * M[0][3] * M[1][2];
            inv.M[1][3] = M[0][0] * M[1][2] * M[2][3] - M[0][0] * M[1][3] * M[2][2] - M[1][0] * M[0][2] * M[2][3] + M[1][0] * M[0][3] * M[2][2] + M[2][0] * M[0][2] * M[1][3] - M[2][0] * M[0][3] * M[1][2];
            inv.M[2][3] = -M[0][0] * M[1][1] * M[2][3] + M[0][0] * M[1][3] * M[2][1] + M[1][0] * M[0][1] * M[2][3] - M[1][0] * M[0][3] * M[2][1] - M[2][0] * M[0][1] * M[1][3] + M[2][0] * M[0][3] * M[1][1];
            inv.M[3][3] = M[0][0] * M[1][1] * M[2][2] - M[0][0] * M[1][2] * M[2][1] - M[1][0] * M[0][1] * M[2][2] + M[1][0] * M[0][2] * M[2][1] + M[2][0] * M[0][1] * M[1][2] - M[2][0] * M[0][2] * M[1][1];

            float32 det = M[0][0] * inv.M[0][0] + M[0][1] * inv.M[1][0] + M[0][2] * inv.M[2][0] + M[0][3] * inv.M[3][0];
            if (std::abs(det) < 1e-6f) return Identity();

            float32 invDet = 1.0f / det;
            for (int row = 0; row < 4; row++)
                for (int col = 0; col < 4; col++)
                    inv.M[row][col] *= invDet;

            return inv;
        }
    };
}
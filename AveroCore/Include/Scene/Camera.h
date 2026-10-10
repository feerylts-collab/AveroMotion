#pragma once

#include "Core/AveroTypes.h"
#include "Math/Matrix4x4.h"

#include <cmath>

namespace Avero::Scene
{
    using Avero::Core::float32;
    using Avero::Core::uint8;

    enum class ProjectionType : uint8
    {
        Perspective = 0,
        Orthographic
    };

    struct Camera
    {
        float32 FOV = 60.0f;
        float32 NearClip = 0.1f;
        float32 FarClip = 1000.0f;
        float32 AspectRatio = 16.0f / 9.0f;
        ProjectionType Type = ProjectionType::Perspective;

        Camera() = default;

        Math::Matrix4x4 GetProjectionMatrix() const
        {
            if (Type == ProjectionType::Perspective)
            {
                return Math::Matrix4x4::Perspective(FOV * Avero::Core::PI / 180.0f, AspectRatio, NearClip, FarClip);
            }
            else
            {
                float32 size = NearClip * std::tan(FOV * 0.5f * Avero::Core::PI / 180.0f);
                return Math::Matrix4x4::Orthographic(-size * AspectRatio, size * AspectRatio, -size, size, NearClip, FarClip);
            }
        }

        Math::Matrix4x4 GetViewMatrix(const Math::Matrix4x4& worldTransform) const
        {
            return worldTransform.Inverse();
        }
    };
}
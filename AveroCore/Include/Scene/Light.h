#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector3.h"

namespace Avero::Scene
{
    using Avero::Core::float32;
    using Avero::Core::uint8;

    enum class LightType : uint8
    {
        Directional = 0,
        Point,
        Spot
    };

    struct Light
    {
        LightType Type = LightType::Directional;
        Math::Vector3 Color = Math::Vector3(1.0f, 1.0f, 1.0f);
        float32 Intensity = 1.0f;
        float32 Range = 10.0f;
        float32 SpotAngle = 45.0f;

        Light() = default;
    };
}
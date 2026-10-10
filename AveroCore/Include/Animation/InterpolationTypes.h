#pragma once

#include "Core/AveroTypes.h"

namespace Avero::Animation
{
    using Avero::Core::uint8;

    enum class InterpolationType : uint8
    {
        Linear = 0,
        Hold,
        Bezier
    };

    enum class TangentMode : uint8
    {
        Auto = 0,   // Smooth tangent (calculated automatically)
        Linear,     // Straight line to next keyframe
        Hold,       // Step function (value holds until next keyframe)
        Free        // Custom Bezier handles
    };
}
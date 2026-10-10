#pragma once

#include "Core/Time.h"
#include "Animation/InterpolationTypes.h"
#include "Math/Vector2.h"

namespace Avero::Animation
{
    template<typename T>
    struct Keyframe
    {
        Core::Time Time;
        T Value;

        InterpolationType Interpolation = InterpolationType::Bezier;
        TangentMode InTangentMode = TangentMode::Auto;
        TangentMode OutTangentMode = TangentMode::Auto;

        // Normalized handles [0, 1] for X, relative for Y.
        // P0=(0,0), P1=OutTangent, P2=InTangent, P3=(1,1)
        Math::Vector2 InTangent;
        Math::Vector2 OutTangent;

        Keyframe() = default;

        Keyframe(Core::Time time, T value, InterpolationType type = InterpolationType::Linear)
            : Time(time), Value(value), Interpolation(type)
        {
        }

        Keyframe(Core::Time time, T value, const Math::Vector2& outTangent, const Math::Vector2& inTangent)
            : Time(time), Value(value), Interpolation(InterpolationType::Bezier),
            InTangentMode(TangentMode::Free), OutTangentMode(TangentMode::Free),
            InTangent(inTangent), OutTangent(outTangent)
        {
        }
    };
}
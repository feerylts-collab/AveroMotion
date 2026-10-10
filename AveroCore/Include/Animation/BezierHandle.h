#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector2.h"

namespace Avero::Animation
{
    using Avero::Core::float32;

    struct BezierHandle
    {
        float32 X = 0.0f;
        float32 Y = 0.0f;

        constexpr BezierHandle() = default;
        constexpr BezierHandle(float32 x, float32 y) : X(x), Y(y) {}

        static constexpr BezierHandle Linear()
        {
            return BezierHandle(0.0f, 0.0f);
        }

        static constexpr BezierHandle EaseIn()
        {
            return BezierHandle(0.42f, 0.0f);
        }

        static constexpr BezierHandle EaseOut()
        {
            return BezierHandle(0.58f, 1.0f);
        }

        static constexpr BezierHandle EaseInOut()
        {
            return BezierHandle(0.42f, 0.0f);
        }
    };
}
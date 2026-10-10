#pragma once

#include "Core/AveroTypes.h"
#include <cmath>

namespace Avero::Animation
{
    using Avero::Core::float32;

    class EasingFunctions
    {
    public:
        static float32 Linear(float32 t)
        {
            return t;
        }

        static float32 EaseIn(float32 t)
        {
            return t * t;
        }

        static float32 EaseOut(float32 t)
        {
            return t * (2.0f - t);
        }

        static float32 EaseInOut(float32 t)
        {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }

        static float32 EaseInQuad(float32 t)
        {
            return t * t;
        }

        static float32 EaseOutQuad(float32 t)
        {
            return t * (2.0f - t);
        }

        static float32 EaseInOutQuad(float32 t)
        {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }

        static float32 EaseInCubic(float32 t)
        {
            return t * t * t;
        }

        static float32 EaseOutCubic(float32 t)
        {
            t -= 1.0f;
            return t * t * t + 1.0f;
        }

        static float32 EaseInOutCubic(float32 t)
        {
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        }
    };
}
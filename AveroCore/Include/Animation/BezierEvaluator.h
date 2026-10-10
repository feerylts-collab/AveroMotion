#pragma once

#include "Core/AveroTypes.h"
#include "Math/Vector2.h"
#include <cmath>
#include <algorithm>

namespace Avero::Animation
{
    using Avero::Core::float32;

    class BezierEvaluator
    {
    public:
        // Solves for t given x in a cubic bezier curve defined by P1(x1,y1) and P2(x2,y2).
        // P0 is (0,0) and P3 is (1,1).
        static float32 SolveCubicBezierX(float32 x, float32 x1, float32 x2)
        {
            // Newton-Raphson iteration
            float32 t = x;
            for (int i = 0; i < 8; ++i)
            {
                float32 currentX = SampleCurveX(t, x1, x2);
                float32 currentSlope = SampleCurveSlope(t, x1, x2);

                if (std::abs(currentSlope) < 1e-6f) break;

                t -= (currentX - x) / currentSlope;
                t = std::clamp(t, 0.0f, 1.0f);
            }
            return t;
        }

        static float32 EvaluateCubicBezierY(float32 t, float32 y1, float32 y2)
        {
            float32 mt = 1.0f - t;
            return 3.0f * mt * mt * t * y1 + 3.0f * mt * t * t * y2 + t * t * t;
        }

    private:
        static float32 SampleCurveX(float32 t, float32 x1, float32 x2)
        {
            float32 mt = 1.0f - t;
            return 3.0f * mt * mt * t * x1 + 3.0f * mt * t * t * x2 + t * t * t;
        }

        static float32 SampleCurveSlope(float32 t, float32 x1, float32 x2)
        {
            float32 mt = 1.0f - t;
            return 3.0f * mt * mt * x1 + 6.0f * mt * t * (x2 - x1) + 3.0f * t * t * (1.0f - x2);
        }
    };
}
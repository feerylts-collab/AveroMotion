#pragma once

#include "Core/AveroTypes.h"
#include "Core/Time.h"
#include "Animation/Keyframe.h"
#include "Animation/BezierEvaluator.h"

#include <vector>
#include <algorithm>
#include <type_traits>

namespace Avero::Animation
{
    using Avero::Core::int64;

    template<typename T>
    class AnimationTrack
    {
    public:
        std::vector<Keyframe<T>> Keyframes;
        mutable int64 CachedSegmentIndex = -1;

        void AddKeyframe(const Keyframe<T>& keyframe)
        {
            Keyframes.push_back(keyframe);
            std::sort(Keyframes.begin(), Keyframes.end(),
                [](const Keyframe<T>& a, const Keyframe<T>& b)
                {
                    return a.Time < b.Time;
                });
            CachedSegmentIndex = -1;
        }

        void RemoveKeyframe(Core::Time time)
        {
            Keyframes.erase(
                std::remove_if(Keyframes.begin(), Keyframes.end(),
                    [time](const Keyframe<T>& kf) { return kf.Time == time; }),
                Keyframes.end()
            );
            CachedSegmentIndex = -1;
        }

        T Evaluate(Core::Time time) const
        {
            if (Keyframes.empty()) return T{};

            if (time <= Keyframes.front().Time) return Keyframes.front().Value;
            if (time >= Keyframes.back().Time) return Keyframes.back().Value;

            int64 segmentIndex = FindSegment(time);

            const auto& kf1 = Keyframes[segmentIndex];
            const auto& kf2 = Keyframes[segmentIndex + 1];

            return Interpolate(kf1, kf2, time);
        }

    private:
        int64 FindSegment(Core::Time time) const
        {
            if (CachedSegmentIndex >= 0 && CachedSegmentIndex < static_cast<int64>(Keyframes.size()) - 1)
            {
                const auto& kf1 = Keyframes[CachedSegmentIndex];
                const auto& kf2 = Keyframes[CachedSegmentIndex + 1];

                if (time >= kf1.Time && time <= kf2.Time)
                {
                    return CachedSegmentIndex;
                }
            }

            auto it = std::upper_bound(
                Keyframes.begin(),
                Keyframes.end(),
                time,
                [](Core::Time t, const Keyframe<T>& kf) { return t < kf.Time; }
            );

            int64 index = static_cast<int64>(it - Keyframes.begin()) - 1;
            if (index < 0) index = 0;
            if (index >= static_cast<int64>(Keyframes.size()) - 1)
            {
                index = static_cast<int64>(Keyframes.size()) - 2;
            }

            CachedSegmentIndex = index;
            return index;
        }

        T Interpolate(const Keyframe<T>& kf1, const Keyframe<T>& kf2, Core::Time time) const
        {
            double duration = (kf2.Time - kf1.Time).ToSeconds();
            if (duration <= 0.0) return kf1.Value;

            double elapsed = (time - kf1.Time).ToSeconds();
            float u = static_cast<float>(elapsed / duration);

            if (kf1.Interpolation == InterpolationType::Hold || kf1.OutTangentMode == TangentMode::Hold)
            {
                return kf1.Value;
            }

            if (kf1.Interpolation == InterpolationType::Linear ||
                kf1.OutTangentMode == TangentMode::Linear)
            {
                return Lerp(kf1.Value, kf2.Value, u);
            }

            if (kf1.Interpolation == InterpolationType::Bezier)
            {
                float x1 = kf1.OutTangent.X;
                float y1 = kf1.OutTangent.Y;
                float x2 = kf2.InTangent.X;
                float y2 = kf2.InTangent.Y;

                float t = BezierEvaluator::SolveCubicBezierX(u, x1, x2);
                float valueProgress = BezierEvaluator::EvaluateCubicBezierY(t, y1, y2);

                return Lerp(kf1.Value, kf2.Value, valueProgress);
            }

            return Lerp(kf1.Value, kf2.Value, u);
        }

        T Lerp(const T& a, const T& b, float t) const
        {
            if constexpr (std::is_same_v<T, float>)
            {
                return a + (b - a) * t;
            }
            else if constexpr (std::is_same_v<T, Math::Vector2>)
            {
                return a.Lerp(b, t);
            }
            else if constexpr (std::is_same_v<T, Math::Vector3>)
            {
                return a.Lerp(b, t);
            }
            else if constexpr (std::is_same_v<T, Math::Vector4>)
            {
                return a.Lerp(b, t);
            }
            else
            {
                return a;
            }
        }
    };
}